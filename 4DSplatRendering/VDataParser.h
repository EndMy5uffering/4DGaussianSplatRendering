#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "glm/glm.hpp"


namespace VData 
{
    struct VSplatData {
        glm::vec3 pos;
        glm::vec4 color;
        glm::mat4 cov;
    };


	std::vector<glm::mat3> parse(const std::string& path) 
	{
		std::vector<std::string> words;
		std::ifstream file(path);

        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) { // Read file line by line
                std::istringstream iss(line);
                std::string word;
                while (iss >> word) { // Split line at spaces
                    words.push_back(word); // Save each string into vector
                }
            }
            file.close();
        }
        else {
            std::cerr << "Unable to open file: " << path << std::endl;
            return std::vector<glm::mat3>{};
        }

        std::vector<glm::mat3> res;
        for (size_t i = 0; i < words.size(); i += 6) 
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

    std::vector<VSplatData> parse_splat_data(const std::string& path)
    {
        std::vector<std::string> words;
        std::ifstream file(path);

        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) { // Read file line by line
                std::istringstream iss(line);
                std::string word;
                while (iss >> word) { // Split line at spaces
                    words.push_back(word); // Save each string into vector
                }
            }
            file.close();
        }
        else {
            std::cerr << "Unable to open file: " << path << std::endl;
            return std::vector<VSplatData>{};
        }

        std::vector<VSplatData> res;
        for (size_t i = 0; i < words.size(); i += 23)
        {
            res.push_back(
                { 
                    glm::vec3 //POS
                    {
                        std::stof(words[i]),
                        std::stof(words[i+1]),
                        std::stof(words[i+2]),
                    },
                    glm::vec4 //COL
                    {
                        std::stof(words[i + 3]),
                        std::stof(words[i + 4]),
                        std::stof(words[i + 5]),
                        std::stof(words[i + 6]),
                    },
                    glm::mat4 //COV
                    {
                        std::stof(words[i + 7]),
                        std::stof(words[i + 8]),
                        std::stof(words[i + 9]),
                        std::stof(words[i + 10]),

                        std::stof(words[i + 11]),
                        std::stof(words[i + 12]),
                        std::stof(words[i + 13]),
                        std::stof(words[i + 14]),

                        std::stof(words[i + 15]),
                        std::stof(words[i + 16]),
                        std::stof(words[i + 17]),
                        std::stof(words[i + 18]),

                        std::stof(words[i + 19]),
                        std::stof(words[i + 20]),
                        std::stof(words[i + 21]),
                        std::stof(words[i + 22]),
                    }
                }
            );
        }
        return res;
    }

}