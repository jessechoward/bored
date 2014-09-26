#include "Message.h"

namespace bored
{
	std::string GetJSON(PropertyTree &tree)
	{
		std::ostringstream str;
		write_json(str,tree,false);
		return str.str();
	}

	bool ValidateJSON(const std::string &json)
	{
				long curly_count, square_count, quote_count;
		const char *p;
		bool escape = false;

		// quick and ugly parse
		for ( p = json.c_str(); *p; ++p )
		{
			switch(*p)
			{
			case '{':
				if ( !escape )
					++curly_count;
				else
					escape = false;
				break;
			case '}':
				if ( !escape )
					--curly_count;
				else
					escape = false;
				break;
			case '[':
				if ( !escape )
					++square_count;
				else
					escape = false;
				break;
			case ']':
				if ( !escape )
					--square_count;
				else
					escape = false;
				break;
			case '"':
				if ( !escape )
					++quote_count;
				else
					escape = false;
				break;
			case '\\':
				escape = !escape;
				break;
			default:
				break;
			}
		}

		if ( !curly_count || !square_count || quote_count%2==1 )
		{
			std::cerr << __FUNCTIONW__ << " invalid JSON. curly:" << curly_count << " square:" << square_count << " quote:" << quote_count << std::endl;
			return false;
		}

		return true;
	}

	bool ParseJSON(const std::string &json, PropertyTree &tree)
	{
		// quick validation...
		if ( !ValidateJSON(json) )
			return false;

		// okay - rudamentary validation done - send it along to the parser
		std::istringstream str(json);
		read_json(str,tree);

		return true;
	}
};