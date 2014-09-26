#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <queue>
#include <list>
#include <cstdlib>

#include <SFML/Graphics.hpp>

namespace bored
{
	sf::Font	DefaultFont;
	sf::Texture	Background,Background2;
	sf::Sprite	StarField,StarField2;

	void Initialize()
	{
		srand(time(NULL));
		DefaultFont.loadFromFile("default.ttf");
		Background.loadFromFile("starsBackground.png");
		StarField.setTexture(Background);
		StarField.setTextureRect(sf::IntRect(sf::Vector2i(rand()%(3000-1024),rand()%(720-600)), sf::Vector2i(1024,600)));

		//Background2.loadFromFile("starsBackground2.png");
		//StarField2.setTexture(Background2);
		//StarField2.setTextureRect(sf::IntRect(sf::Vector2i(rand()%(3000-1128),rand()%(720-664)), sf::Vector2i(1128,664)));
		//StarField2.setOrigin(512.0f,300.0f);
		//StarField2.setPosition(512.0f,300.0f);
	}

	float RandomFloat()
	{
		return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	}

	float RandomFloatRange(float lo, float hi)
	{
		return lo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(hi-lo)));
	}

	class Particle : public sf::Drawable
	{
	private:
		virtual void draw(sf::RenderTarget &target,sf::RenderStates states)const=0;

	public:
		sf::Vector2f	velocity;	// the velocity to update
		sf::Clock		timer;		// the timer to keep track of lifespan
		sf::Uint32		lifetime;	// maximum lifespan in milliseconds

		virtual void Update()=0;
		virtual bool Valid()=0;
	};

	class Bullet : public Particle
	{
	private:
		sf::CircleShape circle;
		void draw(sf::RenderTarget &target,sf::RenderStates states)const{ target.draw(circle); }

		float calculateDamage()
		{
			if ( lifetime != 0 )
			{
				float dam = damage/(float)lifetime;
				return dam * (float)(lifetime - timer.getElapsedTime().asMilliseconds());
			}

			return 0.0f;
		}

	public:

		float damage;

		void Initiate(sf::Vector2f origin, sf::Vector2f direction, sf::Uint32 life)
		{
			static const float speed = 0.35f;
			static const float radius = 1.5f;

			circle.setPosition(origin);
			circle.setRadius(radius);
			circle.setOrigin(radius,radius);
			circle.setFillColor(sf::Color::White);

			velocity = direction;
			velocity.x *= speed;
			velocity.y *= speed;

			lifetime = life;
			timer.restart();
		}

		virtual void Update()
		{
			static const sf::FloatRect screen = sf::FloatRect(sf::Vector2f(0,0),sf::Vector2f(1024,600));

			if ( Valid() )
			{
				// recalculate the damage
				damage = calculateDamage();
				circle.move(velocity);

				// invalidate the bullet if it goes off screen
				if ( !screen.contains(circle.getPosition()) )
					lifetime = 0; 
			}
		}

		virtual bool Valid()
		{
			if ( !lifetime || timer.getElapsedTime().asMilliseconds() >= lifetime )
				return false;

			return true;
		}

		virtual bool CheckCollision( const sf::FloatRect &rect )
		{
			// first check the basic stuff. We won't get too worried about "exactness"
			if ( rect.contains(circle.getPosition()) || circle.getGlobalBounds().intersects(rect) )
			{
				// invalidate the bullet
				lifetime = 0;
				return true;
			}

			return false;
		}
	};

	class BulletEmitter
	{
	public:
		std::list<Bullet*> particles;
		sf::Clock last_particle;

		void Update()
		{
			if ( particles.empty() )
				return;

			std::list<Bullet*>::iterator iter,next;

			for (iter = particles.begin(); iter != particles.end(); iter = next)
			{
				next = iter;
				++next;

				if ( !(*iter)->Valid() )
				{
					Bullet *bullet = (*iter);
					particles.erase(iter);
					delete bullet;
					bullet = NULL;
				}
				else
				{
					(*iter)->Update();
				}
			}			
		}

		void NewParticle(sf::Vector2f location, sf::Vector2f direction, sf::Uint32 life)
		{
			if ( last_particle.getElapsedTime().asMilliseconds() < 5 )
				return;

			Bullet *particle = new Bullet();

			particle->Initiate(location,direction,life);
			particles.push_back(particle);

			last_particle.restart();
		}

		void DrawParticles(sf::RenderWindow &window)
		{
			for (std::list<Bullet*>::iterator iter = particles.begin(); iter != particles.end(); ++iter)
			{
				if ( (*iter)->Valid() )
					window.draw(*(*iter));
			}
		}
	};


	class Smoke : public Particle
	{
	private:
		sf::CircleShape circle;
		void draw(sf::RenderTarget &target,sf::RenderStates states)const{ target.draw(circle); }

		sf::Vector2f RandomizeLocation( sf::Vector2f &location, sf::Uint32 max_pixels )
		{
			int xsign = rand() % 2;
			int ysign = rand() % 2;
			float x = location.x + (bored::RandomFloatRange(0.0f,max_pixels)*(xsign?1.0f:-1.0f));
			float y = location.y + (bored::RandomFloatRange(0.0f,max_pixels)*(ysign?1.0f:-1.0f));

			return sf::Vector2f(x,y);
		}

	public:
		void Initiate(sf::Vector2f location, sf::Vector2f initial_vel, sf::Uint32 life)
		{
			if ( rand() % 2 )
				circle.setPosition(RandomizeLocation(location,2));
			else
				circle.setPosition(location);

			circle.setRadius(2.0f+(rand()%5));
			circle.setOrigin(circle.getRadius(),circle.getRadius());
			circle.setFillColor(sf::Color(255,128,50));
			lifetime = life;
			timer.restart();
			velocity.x = initial_vel.x;
			velocity.y = initial_vel.y;
		}

		bool Valid()
		{
			return (timer.getElapsedTime().asMilliseconds()>lifetime?false:true);
		}

		void Update()
		{
			circle.move(velocity.x,velocity.y);

			if( rand() % 1000 == 0 )
				circle.scale(bored::RandomFloat(),bored::RandomFloat());
			
			if ( Valid() )
			{
				sf::Color bg(circle.getFillColor());
				bg.a = ( (lifetime/256) * (lifetime-timer.getElapsedTime().asMilliseconds()) );
				circle.setFillColor(bg);
			}
		}

	};

	struct SmokeEmiter
	{
		std::list<Smoke*> particles;
		sf::Clock last_particle;

		void Update()
		{
			if ( particles.empty() )
				return;

			std::list<Smoke*>::iterator iter,next;

			for (iter = particles.begin(); iter != particles.end(); iter = next)
			{
				next = iter;
				++next;

				if ( !(*iter)->Valid() )
				{
					Smoke *smoke = (*iter);
					particles.erase(iter);
					delete smoke;
					smoke = NULL;
				}
				else
				{
					(*iter)->Update();
				}
			}			
		}

		void NewParticle(sf::Vector2f location, sf::Vector2f initial_vel, sf::Uint32 life)
		{
			if ( last_particle.getElapsedTime().asMilliseconds() < 5 )
				return;

			Smoke *particle = new Smoke();

			particle->Initiate(location,initial_vel,life);
			particles.push_back(particle);

			last_particle.restart();
		}

		void DrawParticles(sf::RenderWindow &window)
		{
			for (std::list<Smoke*>::iterator iter = particles.begin(); iter != particles.end(); ++iter)
					window.draw(*(*iter));
		}
	};

	struct StatusMeter
	{
		float min, max, current;
		sf::Text	status;
		sf::RectangleShape outline;
		sf::RectangleShape fill;

		void Initialize(sf::Vector2f &location, float base, float initial, float top)
		{
			min = base;
			current = initial;
			max = top;

			outline.setPosition(location);
			outline.setSize(sf::Vector2f(102.0f, 18.0f));
			outline.setOutlineThickness(1.5f);
			outline.setOutlineColor(sf::Color::White);
			outline.setFillColor(sf::Color::Red);
			fill.setPosition(location.x+1, location.y+1);
			fill.setSize(sf::Vector2f(100.0f, 16.0f));
			fill.setOutlineColor(sf::Color::Green);
			fill.setFillColor(sf::Color::Green);

			status.setFont(bored::DefaultFont);
			status.setCharacterSize(16);
			status.setColor(sf::Color::White);
			status.setPosition(location.x, location.y+22);

			SetValue(initial);
		}

		void SetValue(float value)
		{
			static char buffer[256];

			current = value;
			sprintf( buffer, "%.2f/%.2f", current, max );
			status.setString(sf::String(std::string(buffer)));
			fill.setSize(sf::Vector2f(100*(current/max),16.0f));

			//std::cout << buffer << std::endl;
		}

		void DrawMeter( sf::RenderWindow &window )
		{
			window.draw(outline);
			window.draw(fill);
			window.draw(status);
		}
	};

	struct Stats
	{
		sf::Uint32		kills;
		sf::Uint32		deaths;
	};

	struct Ship
	{
		std::string		uuid;		// uinique identifier
		sf::Sprite		sprite;		// the visual sprite
		sf::Vector2f	velocity;	// the velocity vector (true x/y, not direction/speed)
		sf::Int8		accelerate;	// are we applying thrusters or not (0=no, non-zero=yes)
		sf::Int8		rotate;		// 1=clockwise, 0 no rotate, -1 counter-clockwise
		sf::Int8		fire;		// non-zero is trigger happy!
		sf::Clock		last_fire;	// how long since last bullet
		float			power; // how much power the thrusters have
		StatusMeter		power_status; // status meter
		StatusMeter		velocity_status; // status meter
		SmokeEmiter	emiter; // the smoke emiter!
		BulletEmitter gun; // the BOOM!

		Ship(const std::string &uid):uuid(uid)
		{
			velocity.x = velocity.y = 0.0f;
			accelerate = 0;
			fire = 0;
			rotate = 0;
			power = 50.0f;
			power_status.Initialize(sf::Vector2f(32,32),0.0f,power,100.0f);
			velocity_status.Initialize(sf::Vector2f(32,76),0.0f,power,100.0f);
		}


		void Update( const sf::Time &delta )
		{
			static const float acceleration = 0.25;
			static const float RAD = 3.14159f / 180.0f; // radians is actually degrees multiplied by this result
			
			if ( rotate )
				sprite.rotate(rotate*(delta.asSeconds()*360.0f)); // time it takes to rotate

			if ( accelerate )
			{
				// the direction
				sf::Vector2f force;
				// convert degrees to radians
				float radians = sprite.getRotation() * RAD;
				float reverse_radians = (sprite.getRotation()-180.0f) * RAD;
				// speed may not be the correct term. I may have it swapped with acceleration
				float speed = acceleration*delta.asSeconds()*accelerate;
				// decrease the ships power
				power -= delta.asSeconds()*50.0f;
				power = std::max<float>(0.0f,power);

				// get the direction and magnitude of the thrusting force we will apply
				if ( accelerate > 0 )
				{
					force.x = cos(radians) * speed;
					force.y = sin(radians) * speed;
				}
				else
				{
					force.x = cos(reverse_radians) * speed;
					force.y = sin(reverse_radians) * speed;
				}

				// apply the thrusting force to the current velocity vector
				if ( accelerate )
					this->velocity += force;


				// update the status bars
				power_status.SetValue(power);
				velocity_status.SetValue(sqrt( (velocity.x*velocity.x)+(velocity.y*velocity.y))*1000);

				float reverse_angle = sprite.getRotation() - 180.0f;
				if ( reverse_angle < 0.0f )
					reverse_angle += 360.0f;
				reverse_angle *= RAD;
				sf::Vector2f smoke_vel;
				/*smoke_vel.x = cos(radians)*speed*(-1000.0f);
				smoke_vel.y = sin(radians)*speed*(-1000.0f);*/
				smoke_vel.x = cos(radians)*speed;
				smoke_vel.y = sin(radians)*speed;
				if ( accelerate > 0 )
					smoke_vel -= this->velocity;
				else
					smoke_vel += this->velocity;


				emiter.NewParticle(sprite.getPosition()+smoke_vel,smoke_vel,500);
			}
			else
			{
				// add the ships  power back
				power += delta.asSeconds()*25.0f;
				power = std::min<float>(100.0f,power);

				// update the power bar
				power_status.SetValue(power);
				// no need to update the velocity bar because it hasn't changed
				//velocity_status.SetValue(sqrt((velocity.x*velocity.x)+(velocity.y*velocity.y)));
			}

			if ( fire )
			{
				// cannot fire faster than 10 bullets per second or have more than 6 bullets on the screen at once
				if ( gun.particles.size() < 6 && last_fire.getElapsedTime().asMilliseconds() >= 100 )
				{
					float radians = sprite.getRotation() * RAD;
					gun.NewParticle(sprite.getPosition(),sf::Vector2f(cos(radians),sin(radians)), 1000);
					last_fire.restart();
				}
			}

			// actually move the ship
			sprite.move(velocity);

			// hack - needs to be moved/checked somewhere else
			// locked into current screen size
			if ( sprite.getPosition().x <=0 || sprite.getPosition().x >= 1024.0f )
				sprite.setPosition( abs(sprite.getPosition().x - 1024.0f), sprite.getPosition().y );
			if ( sprite.getPosition().y <=0 || sprite.getPosition().y >= 600.0f )
				sprite.setPosition( sprite.getPosition().x, abs(sprite.getPosition().y - 600.0f) );

			// update particles
			emiter.Update();
			// update bullets!
			gun.Update();
		}
	};

	typedef std::map<std::string,Ship*>	ShipMap;
	typedef std::list<Ship*>			ShipList;

	struct ShipManager
	{
		sf::Texture image;
		ShipMap		ship_map;
		ShipList	free_ships;

		Ship *NewShip(const std::string &uid)
		{
			// load the image if we haven't already
			if ( image.getSize().x < 1 )
				image.loadFromFile("ship16.png");

			// don't create a newship if we already have it (duplicate messages)
			if ( ship_map.find(uid) != ship_map.end() )
				return ship_map[uid];

			// create the new ship structure
			Ship *pNewShip = new Ship(uid);
			// add to the map
			ship_map[uid] = pNewShip;

			pNewShip->sprite.setTexture(image);
			pNewShip->sprite.setOrigin(8,8);
			pNewShip->sprite.setPosition(512,300);

			return pNewShip;
		}

		void FreeShip(const std::string &uid)
		{
			// don't try to delete something that isn't there or already marked for freeing
			if ( ship_map.find(uid) == ship_map.end() )
				return;

			free_ships.push_back(ship_map[uid]);
		}

		void FreeShip(Ship *free_ship)
		{
			// don't try to delete something that isn't there or already marked for freeing
			if ( !free_ship || free_ship->uuid.empty() || ship_map.find(free_ship->uuid) == ship_map.end() )
				return;

			free_ships.push_back(free_ship);
		}

		void Cleanup()
		{
			// free up dead ships
			while( !free_ships.empty() )
			{
				Ship *pShip = free_ships.front();
				// remove from the free up list - no longer needed
				free_ships.pop_front();
				std::string uid = pShip->uuid;
				
				// remove it from the map
				if ( !uid.empty() && ship_map.find(uid) != ship_map.end() )
					ship_map.erase(uid);

				// free up the memory
				delete pShip;
				pShip = NULL;				
			}
		}

		Ship *getShip( const std::string &uid ) const
		{
			if ( !uid.empty() && ship_map.find(uid) != ship_map.end() )
				return ship_map.at(uid);

			return NULL;
		}

		void Update( const sf::Time &delta )
		{
			// the maximum time slice to allow processing on
			// prevents running through things on long delays
			static const sf::Time slice = sf::milliseconds(10);
			sf::Time frame = delta;

			// perform the calculations to catch up in fixed time slices
			while( frame > slice )
			{
				for( ShipMap::iterator iter = ship_map.begin(); iter != ship_map.end(); ++iter )
					(iter->second)->Update(slice);

				frame -= slice;
			}

			// physics in small parts...
			for( ShipMap::iterator iter = ship_map.begin(); iter != ship_map.end(); ++iter )
					(iter->second)->Update(frame);

			Cleanup();
		}

		void DrawShips(sf::RenderWindow &window)
		{
			for( ShipMap::iterator iter = ship_map.begin(); iter != ship_map.end(); ++iter )
			{
				(iter->second)->emiter.DrawParticles(window);
				(iter->second)->gun.DrawParticles(window);
				window.draw((iter->second)->sprite);
			}
		}
	};

	struct Zone
	{
		sf::RenderWindow	window;
		std::string			name;
		sf::Uint32			pixel_width;
		sf::Uint32			pixel_height;
		std::map<std::string,Stats> scoreboard;
		ShipManager			manager;
		sf::Clock			clock;

		void Create( sf::Uint32 width, sf::Uint32 height, const std::string &title )
		{
			window.create(sf::VideoMode(width,height), title);
			name = title;
			pixel_width = width;
			pixel_height = height;

			// we don't want multiple key events for the same press
			window.setKeyRepeatEnabled(false);
		}

		bool Contains( sf::FloatRect &rect )
		{
			// checks for collision of the rectangles. Note that on the line is out of bounds.
			if ( rect.left <= 0.0f || rect.left+rect.width >= pixel_width ||
				 rect.top <= 0.0f || rect.top+rect.height >= pixel_height )
				 return false;
		}

		bool Contains( sf::IntRect &rect )
		{
			// checks for collision of the rectangles. Note that on the line is out of bounds.
			if ( rect.left <= 0 || rect.left+rect.width >= pixel_width ||
				 rect.top <= 0 || rect.top+rect.height >= pixel_height )
				 return false;
		}

		// handle window and keyboard events
		// the ship pointer is to the currently controlled ship.
		bool HandleEvents( Ship *pShip )
		{
			sf::Event event;

			while( this->window.pollEvent(event) )
			{
				switch( event.type )
				{
				case sf::Event::Closed:
					manager.FreeShip(pShip);
					window.close();
					return false; // we need to close the window and exit

				case sf::Event::KeyPressed:
					switch( event.key.code )
					{
					case sf::Keyboard::Up:
						pShip->accelerate += 1;
						std::cout << "u";
						break;
					case sf::Keyboard::Down:
						pShip->accelerate -= 1;
						std::cout << "d";
						break;
					case sf::Keyboard::Right:
						pShip->rotate += 1;
						std::cout << "r";
						break;
					case sf::Keyboard::Left:
						pShip->rotate -= 1;
						std::cout << "l";
						break;
					case sf::Keyboard::Space:
						pShip->fire = 1;
						std::cout << "F";
						break;
					default:
						break;
					}
					break;

				case sf::Event::KeyReleased:
					switch( event.key.code )
					{
					case sf::Keyboard::Up:
						pShip->accelerate -= 1;
						break;
					case sf::Keyboard::Down:
						pShip->accelerate += 1;
						break;
					case sf::Keyboard::Right:
						pShip->rotate -= 1;
						break;
					case sf::Keyboard::Left:
						pShip->rotate += 1;
						break;
					case sf::Keyboard::Space:
						pShip->fire = 0;
						break;
					default:
						break;
					}
					break;

				default:
					break;
				}
			}

			return true;
		}

		

		void Loop(Ship *pShip)
		{
			clock.restart();

			while( HandleEvents(pShip) )
			{
				// do all the physics updates based on elapsed time
				manager.Update(clock.getElapsedTime());
				// make sure we have done all time based calculations before this point
				// reset the timer to now
				clock.restart();
				// clear the back buffer
				// we don't have to do this - in fact we SHOULDN'T do this
				// if our redraw covers the entire screen each frame
				//window.clear();
				// draw background stuff
				window.draw(bored::StarField);
//				bored::StarField2.rotate(clock.getElapsedTime().asSeconds()*100.0f);
//				window.draw(bored::StarField2);
				// draw ships and objects
				manager.DrawShips(window);
				// draw heads up displays / overlays for the ship we are controling
				pShip->power_status.DrawMeter(window);
				pShip->velocity_status.DrawMeter(window);
				// flip the display (assuming double buffer)
				window.display();

				sf::sleep(sf::Time(sf::microseconds(100)));
			}
		}
	};
};

int main()
{
	bored::Initialize();

	bored::Zone zone;
	bored::Ship *pShip;
	std::string name;
	

	std::cout << "What do you want to call your ship?: ";
	std::cin >> name;

	zone.Create(1024,600,"Bored out of my mind");
	pShip = zone.manager.NewShip(name);
	zone.Loop(pShip);

	std::cout << "\n\rExited normally\n\rPress any key to exit." << std::endl;
	
	char c;
	std::cin >> c;

	return -1;
}