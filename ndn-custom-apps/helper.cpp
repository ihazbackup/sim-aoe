#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace std {

class Helper 
{
public:
	// Split string with delimiter
	static vector<string> split(const string& s, const char& delim)
	{
		string buff{""};
		vector<string> v;
	
		for(auto n:s)
		{
			if(n != delim) buff+=n; else
			if(n == delim && buff != "") { v.push_back(buff); buff = ""; }
		}
		if(buff != "") v.push_back(buff);
	
		return v;
	}
	
};

}