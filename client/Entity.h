#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <queue>
#include <SFML/Graphics.hpp>

#include "Message.h"

namespace bored
{

class EntityState
{
public:
	std::string		uid;
	sf::Int8		accelerate;
	sf::Int8		rotate;
	sf::Vector2f	position;
	sf::Vector2f	origin;
	sf::Vector2f	size;
	sf::Vector2f	direction;
	float			rotation;

	virtual void FromMessage( PropertyTree &envelope )
	{
		PropertyTree msg = envelope.get_child("msg",envelope);
		PropertyTree header = envelope.get_child("header",envelope);
		
	}

	virtual std::string BuildMessage()
	{
		PropertyTree msg;
		PropertyTree header;
		PropertyTree envelope;

		msg.add("uid",uid);
		msg.add("accelerate",accelerate);
		msg.add("rotate",rotate);
		msg.add("position.x",position.x);
		msg.add("position.y",position.y);
		msg.add("origin.x",origin.x);
		msg.add("origin.y",origin.y);
		msg.add("size.x",size.x);
		msg.add("size.y",size.y);
		msg.add("direction.x",direction.x);
		msg.add("direction.y",direction.y);

		header.add("type","entitystate");
		header.add("uid",uid);

		envelope.add_child("header",header);
		envelope.add_child("msg",msg);

		return bored::GetJSON(envelope);
	}
};

typedef std::queue<PropertyTree>	MessageQueue;

class Entity : public sf::Drawable
{
private:
	virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const;

protected:
	EntityState	m_state;
	MessageQueue	m_message_queue;

	virtual void HandleMessage();

public:

	Entity(const std::string &uid);
	virtual ~Entity(void);

	virtual void PushMessage( PropertyTree &message );
	virtual void Update( sf::Time &delta );
};

};