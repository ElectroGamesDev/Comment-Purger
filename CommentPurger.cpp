#include <iostream>
#include <conio.h>
#include <vector>
#include <fstream>
#include <filesystem>
#include <string>

bool IsInQuotes(const std::string& line, size_t pos) {
    bool inQuotes = false;

    for (size_t i = 0; i < pos; ++i)
        if (line[i] == '"' || line[i] == '\'')
            inQuotes = !inQuotes;

    return inQuotes;
}

bool PurgeFile(std::filesystem::path path)
{
    std::ifstream file(path);

    if (!file.is_open()) return false;


    bool multiLinedComment = false;
    std::string line;
    std::vector<std::string> modifiedLines;

    if (path.extension() == ".py" || path.extension() == ".rb")
    {
        while (std::getline(file, line))
        {
            size_t singlePos = line.find('#');
            size_t multiPos = line.find("\"\"\"");
            if (singlePos != std::string::npos && !IsInQuotes(line, singlePos))
            {
                modifiedLines.push_back(line.substr(0, singlePos));
            }
            else if (multiPos != std::string::npos && !IsInQuotes(line, multiPos)) multiLinedComment = !multiLinedComment;
            else if (!multiLinedComment) modifiedLines.push_back(line);
        }
    }
    else
    {
        while (std::getline(file, line))
        {
            size_t singlePos = line.find("//");
            size_t multiPos = line.find("/*");
            size_t multiPos2 = line.find("*/");
            if (singlePos != std::string::npos && !IsInQuotes(line, singlePos))
            {
                std::string tempLine = line.substr(0, singlePos);
                tempLine.erase(std::remove_if(tempLine.begin(), tempLine.end(), ::isspace), tempLine.end());
                if (!tempLine.empty()) modifiedLines.push_back(tempLine);
            }
            else if (multiPos != std::string::npos && !IsInQuotes(line, multiPos)) multiLinedComment = true;
            else if (multiPos2 != std::string::npos && !IsInQuotes(line, multiPos2)) multiLinedComment = false;
            else if (!multiLinedComment) modifiedLines.push_back(line);
        }
    }

    file.close();

    std::ofstream outputFile(path);
    if (!outputFile.is_open()) return false;

    for (const std::string& modifiedLine : modifiedLines) {
        outputFile << modifiedLine << std::endl;
    }

    return true;
}

int main()
{
    std::cout << "Drag and drop a file or folder to purge it of comments..." << std::endl;

    std::vector< std::string > files;

    for (int ch = _getch(); ch != '\r'; ch = _getch()) {

        std::string file_name;

        if (ch == '\"')
        {
            while ((ch = _getch()) != '\"')
                file_name += ch;
        }
        else
        {
            file_name += ch;
            while (_kbhit())
                file_name += _getch();
        }

        files.push_back(file_name);
        break;
    }

    std::vector< std::string > filesPurged;

    for (auto& file : files)
    {
        if (std::filesystem::is_directory(file))
        {
            for (const auto& fileFound : std::filesystem::recursive_directory_iterator(file))
                if (PurgeFile(fileFound)) filesPurged.push_back(fileFound.path().string());
        }
        else if (PurgeFile(file)) filesPurged.push_back(file);
    }

    std::cout << "\n---- Files Purged ---" << std::endl;
    for (const std::string file : filesPurged)
        std::cout << file << std::endl;

    std::cout << "\nPress [Enter] To Close" << std::endl;
    std::cin.ignore();
    return 0;
}