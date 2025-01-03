import yaml
import random

with open("example.yaml", "r", encoding="utf-8") as f:
    text = f.read()

schema = yaml.load(text, Loader=yaml.FullLoader)

data = schema.get("data", {})
functions = schema.get("functions", {})

def run(main):
    params = main.get("parameters", None)
    code = main.get("code")

    return_value = None
    exec_line = 0
    while exec_line < len(code):
        line = code[exec_line]
  
        if isinstance(line, str):
            key = line
        else:
            key = list(line.keys())[0]

        if key == "set":
            var = line.get("set", {}).get("var")
            value = run(
                {"parameters": {}, "code": line.get("set", {}).get("value")}
            )
            data[var] = value

        elif key == "print":
            print(line.get("print"))

        elif key == "input":
            var = line.get("input", {}).get("var")
            data[var] = input()

        elif key == "if":
            condition = run({"code": line.get("if", {}).get("condition")})
            if condition:
                run({"parameters": {}, "code": line.get("if", {}).get("then")})
            else:
                run({"parameters": {}, "code": line.get("if", {}).get("else")})

        elif key == "return":

            if "var" in line.get("return", {}):
                return data[line.get("return", {}).get("var")]
            elif "const" in line.get("return", {}):
                return line.get("return", {}).get("const")
            else:
                return return_value

        elif key == "equal":
            line.get("equal", [])[0]
            value1 = run({"code": [line.get("equal", [])[0]]}) 
            value2 = run({"code": [line.get("equal", [])[1]]})
            if value1 == value2:
                return True
            else:
                return False

        elif key == "greater":
            line.get("greater", [])[0]
            value1 = run({"code": [line.get("greater", [])[0]]}) 
            value2 = run({"code": [line.get("greater", [])[1]]})
            if value1 > value2:
                return True
            else:
                return False
            
        elif key == "var":
            return data[line.get("var", "")]
        
        elif key == "const":
            return line.get("const", "")

        elif key == "random":
            m1 = line.get("random", {}).get("min", {})
            m2 = line.get("random", {}).get("max", {})
            return random.randint(m1, m2)
        
        elif key == "int":
            return int(run({"code": line.get("int", [])}))

        elif key == "end":
            exit

        elif key in functions:
            run(functions.get(key, [])) 

        exec_line += 1

run(functions["main"])