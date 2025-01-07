#include <iostream>
#include <fstream>
#include <exception>
#include <string>
#include <map>
#include <vector>
#include <any>
#include <random>
#include <filesystem>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

using namespace std;
using namespace YAML;

using ParametersType = map<string, any>;
using FunctionType = Node;

static map<string, any> g_data;
static map<string, FunctionType> g_functions;

string getFileStem(const string& filePath)
{
    return filesystem::path(filePath).stem().string();
}

void loadSchema(const string& filePath, Node& root, bool isRoot)
{
    // Load the file
    Node doc = LoadFile(filePath);

    // Process includes first
    if (doc["include"])
    {
        for (const auto& inc : doc["include"])
        {
            string includePath = inc.as<string>();
            // Recursively load includes
            loadSchema(includePath, root, false /* definitely not root */);
        }
    }

    // Now merge the rest of the data
    // We'll rename if we're NOT the root file
    string stem = getFileStem(filePath);

    for (auto it = doc.begin(); it != doc.end(); ++it)
    {
        string funcName = it->first.as<string>();
        if (!isRoot)
        {
            // Prepend filename stem plus a dot
            string newName = stem + "." + funcName;
            cout << "name: " << newName << endl;
            root[newName] = it->second;
        }
        else
        {
            // Keep the function name as is, if this is the root
            root[funcName] = it->second;
        }
    }
}

string anyToString(const any &val)
{
    if (val.type() == typeid(string))
        return any_cast<string>(val);
    return "";
}

bool anyEqual(const any &a, const any &b)
{
    // Handle string == string
    if (a.type() == typeid(string) && b.type() == typeid(string))
    {
        return any_cast<string>(a) == any_cast<string>(b);
    }
    // Handle int == int
    if (a.type() == typeid(int) && b.type() == typeid(int))
    {
        return any_cast<int>(a) == any_cast<int>(b);
    }
    return false;
}

bool anyGreater(const any &a, const any &b)
{
    if (a.type() == typeid(int) && b.type() == typeid(int))
    {
        return any_cast<int>(a) > any_cast<int>(b);
    }
    // You could add more checks for double, etc.
    return false;
}

any run(const Node &mainNode)
{
    any return_value;

    // Extract code (list) from the current node
    vector<Node> code;
    if (mainNode["do"])
    {
        for (auto &&item : mainNode["do"])
        {
            code.push_back(item);
        }
    }

    for (Node &line : code)
    {
        string key;

        // If the line is a scalar, treat it like a plain string
        if (line.IsScalar())
        {
            key = line.as<string>();
        }
        // Otherwise, assume it's a map with exactly one key
        else if (line.IsMap())
        {
            // The first key in the map
            key = line.begin()->first.as<string>();
        }
        else
        {
            // Unexpected line format
            continue;
        }

        // --------------------------------------------------
        // replicate your Python logic for each 'key'
        // --------------------------------------------------
        if (key == "set")
        {
            string var = line["set"]["var"].as<string>();
            Node valCode = line["set"]["value"];

            any value = run(Node{
                NodeType::Map});

            {
                Node temp;
                temp["do"] = valCode;
                value = run(temp); // run that code
            }

            // Store into global data
            g_data[var] = value;
        }
        else if (key == "print")
        {
            if (line["print"])
            {
                for (Node &arr : line["print"].as<vector<Node>>())
                {
                    if (arr["const"])
                    {
                        string constVal = arr["const"].as<string>();
                        cout << constVal;
                    }
                    else if (arr["var"])
                    {
                        string varName = arr["var"].as<string>();
                        cout << anyToString(g_data[varName]);
                    }
                }

                cout << endl;
            }
        }
        else if (key == "write")
        {
            if (line["write"])
            {
                for (Node &arr : line["write"].as<vector<Node>>())
                {
                    if (arr["const"])
                    {
                        string constVal = arr["const"].as<string>();
                        cout << constVal;
                    }
                    else if (arr["var"])
                    {
                        string varName = arr["var"].as<string>();
                        cout << anyToString(g_data[varName]);
                    }
                }
            }
        }
        else if (key == "input")
        {
            // line["input"] has "var"
            string var = line["input"]["var"].as<string>();
            string userInput;
            getline(cin, userInput);
            g_data[var] = userInput;
        }
        else if (key == "if")
        {
            Node conditionCode = line["if"]["condition"];
            Node thenCode = line["if"]["then"];
            Node elseCode = line["if"]["else"];

            // Run condition
            {
                Node temp;
                temp["do"] = conditionCode;
                any conditionResult = run(temp);

                bool cond = false;
                if (conditionResult.type() == typeid(bool))
                    cond = any_cast<bool>(conditionResult);

                // If true, run 'then' code, else run 'else' code
                if (cond)
                {
                    Node tempThen;
                    tempThen["do"] = thenCode;
                    run(tempThen);
                }
                else if (elseCode)
                {
                    Node tempElse;
                    tempElse["do"] = elseCode;
                    run(tempElse);
                }
            }
        }

        else if (key == "var")
        {
            if (line["var"])
            {
                string varName = line["var"].as<string>();
                // Return the value stored in g_data
                return g_data[varName];
            }
        }

        else if (key == "const")
        {
            if (line["const"])
            {
                string constVal = line["const"].as<string>();
                return constVal;
            }
        }

        else if (key == "return")
        {
            return return_value;
        }

        else if (key == "equal")
        {
            // line["equal"] is a list of two items
            auto arr = line["equal"];
            if (arr.size() == 2)
            {
                // Evaluate each side
                Node lhsCode, rhsCode;
                lhsCode.push_back(arr[0]);
                rhsCode.push_back(arr[1]);

                // Run them
                Node tempLHS, tempRHS;
                tempLHS["do"] = lhsCode;
                tempRHS["do"] = rhsCode;

                any v1 = run(tempLHS);
                any v2 = run(tempRHS);

                return anyEqual(v1, v2);
            }
        }
        else if (key == "greaterThan")
        {
            // line["greaterThan"] is a list of two items
            auto arr = line["greaterThan"];
            if (arr.size() == 2)
            {
                Node lhsCode, rhsCode;
                lhsCode.push_back(arr[0]);
                rhsCode.push_back(arr[1]);

                Node tempLHS, tempRHS;
                tempLHS["do"] = lhsCode;
                tempRHS["do"] = rhsCode;

                any v1 = run(tempLHS);
                any v2 = run(tempRHS);

                return anyGreater(v1, v2);
            }
        }
        else if (key == "var")
        {
            // line["var"] is the variable name
            string varName = line["var"].as<string>();
            return g_data[varName];
        }
        else if (key == "const")
        {
            // line["const"] is the direct constant
            string c = line["const"].as<string>();
            return c;
        }
        else if (key == "random")
        {
            // line["random"] has "min" and "max"
            int mn = 0, mx = 0;
            if (line["random"]["min"])
                mn = line["random"]["min"].as<int>();
            if (line["random"]["max"])
                mx = line["random"]["max"].as<int>();

            static random_device rd;
            static mt19937 gen(rd());
            uniform_int_distribution<> dist(mn, mx);
            return dist(gen); // returns an int
        }
        else if (key == "int")
        {
            // line["int"] is code to run, then cast to int
            Node valCode = line["int"];
            Node temp;
            temp["do"] = valCode;

            any val = run(temp);
            // Attempt to convert to int
            if (val.type() == typeid(string))
            {
                // Convert from string to int
                return stoi(any_cast<string>(val));
            }
            else if (val.type() == typeid(int))
            {
                return any_cast<int>(val);
            }
            // Add more logic if needed
        }

        else if (key == "while")
        {
            Node conditionCode = line["if"]["condition"];
            Node processCode = line["if"]["do"];

            Node consider;
            consider["do"] = conditionCode;
            any conditionResult = run(consider);

            while (any_cast<bool>(conditionResult))
            {
                Node process;
                process["do"] = conditionCode;
                run(process);
                conditionResult = run(consider);
            }
        }

        else if (key == "end")
        {
            // Equivalent to Python's exit()
            // We'll just break out of the loop
            break;
        }
        else
        {
            // If 'key' is in g_functions, call that function
            if (g_functions.find(key) != g_functions.end())
            {
                // Call run on that function's node
                run(g_functions[key]);
            }
        }
    }

    // Default return
    return return_value;
}

// Main program entry
int main(int argc, char* argv[])
{
    try
    {
        if (argc < 2)
        {
            cerr << "Usage: " << argv[0] << " <yaml-file>\n";
            return 1;
        }

        // 1) Load and merge everything into one big "schema" node
        Node schema;
        loadSchema(argv[1], schema, true /* isRoot */);

        // 2) Extract "data" into g_data
        if (schema["data"])
        {
            for (auto it = schema["data"].begin(); it != schema["data"].end(); ++it)
            {
                string key = it->first.as<string>();
                g_data[key] = it->second.as<string>();
            }
        }

        // 3) Extract "functions" into g_functions
        if (schema["functions"])
        {
            for (auto it = schema["functions"].begin(); it != schema["functions"].end(); ++it)
            {
                string funcName = it->first.as<string>();
                Node funcNode = it->second;
                g_functions[funcName] = funcNode;
            }
        }

        // 4) Finally, run the "main" function
        //    Because the root file keeps its functions as-is, it should still be "main" in the root file
        //    (unless you intentionally changed it).
        if (g_functions.find("main") != g_functions.end())
        {
            run(g_functions["main"]);
        }
        else
        {
            cerr << "No main function found in YAML.\n";
        }
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}