#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "glm/glm.hpp"


namespace VData 
{

	std::vector<glm::mat3> parse(const std::string& path) 
	{
		std::vector<std::string> words; // Vector to store split strings
		std::ifstream file(path); // Open file

        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) { // Read file line by line
                std::istringstream iss(line);
                std::string word;
                while (iss >> word) { // Split line at spaces
                    words.push_back(word); // Save each string into vector
                }
            }
            file.close(); // Close file
        }
        else {
            std::cerr << "Unable to open file: " << path << std::endl;
            return std::vector<glm::mat3>{};
        }

        std::vector<glm::mat3> res;
        for (int i = 0; i < words.size(); i += 6) 
        {
            float x0 = std::stof(words[i]);
            float y0 = std::stof(words[i + 1]);
            float z0 = std::stof(words[i + 2]);
            float x1 = std::stof(words[i + 3]);
            float y1 = std::stof(words[i + 4]);
            float z1 = std::stof(words[i + 5]);
            res.push_back({ x0, y0, z0, x1, y1, z1, 0, 0, 0 });
        }
        return res;
	}

}