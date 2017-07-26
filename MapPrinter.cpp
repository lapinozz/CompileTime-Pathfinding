#include <iostream>
#include <vector>

int main()
{
  // Regex used on regex101.com
	//  .*?(Vec<(\d+), (\d+)>).*?
	//  {$2,$3}, \n

	int mapSize = 8;

	std::vector<char> map = {
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 0,
	0, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 1, 1, 0, 1, 0,
	0, 0, 0, 1, 0, 0, 1, 0,
	0, 0, 0, 1, 0, 0, 1, 0,
	0, 1, 1, 1, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0
	};

	struct Vec
	{
		int x;
		int y;
	};

	std::vector<Vec> path = 
	{
    // Insert Regex formated code here
		{0,0}, 
		{1,0}, 
		{2,0}, 
		{3,0}, 
		{4,0}, 
		{5,0}, 
		{6,0}, 
		{7,0}, 
		{7,1}, 
		{7,2}, 
		{7,3}, 
		{7,4}, 
		{7,5}, 
		{7,6}, 
		{7,7}, 
		{6,7}, 
		{5,7}, 
		{5,6}, 
		{5,5}, 
		{5,4}, 
		{5,3}, 
		{5,2}, 
		{4,2}, 
		{3,2}, 
		{2,2}, 
		{2,3}
	};

	for(char& c : map)
	{
		c = (c == 0 ? ' ' : '#');
	}

	for(const Vec& vec : path)
	{
		map[vec.x + vec.y*mapSize] = '*';
	}

	for(size_t x = 0; x < map.size(); x++)
	{
		if(x % mapSize == 0 && x != 0)
			std::cout << '\n';
			
		std::cout << map[x];
	}
}
