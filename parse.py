import yaml
import random
from typing import Any, Dict, List, Union

# Define type aliases
CodeType = List[Union[str, Dict[str, Any]]]
ParametersType = Dict[str, Any]
FunctionType = Dict[str, Any]
SchemaType = Dict[str, Any]

# Read and parse the YAML file
with open("example.yaml", "r", encoding="utf-8") as f:
    text: str = f.read()

schema: SchemaType = yaml.load(text, Loader=yaml.FullLoader)

data: Dict[str, Any] = schema.get("data", {})
functions: Dict[str, FunctionType] = schema.get("functions", {})

def run(main: Dict[str, Any]) -> Any:
    params: Union[None, ParametersType] = main.get("parameters", None)
    code: CodeType = main.get("code", [])
    
    return_value: Any = None
    exec_line: int = 0

    while exec_line < len(code):
        line: Union[str, Dict[str, Any]] = code[exec_line]
  
        if isinstance(line, str):
            key: str = line
        else:
            key = list(line.keys())[0]

        if key == "set":
            var: str = line.get("set", {}).get("var", "")
            value: Any = run({"parameters": {}, "code": line.get("set", {}).get("value", [])})
            data[var] = value

        elif key == "print":
            print(line.get("print"))

        elif key == "input":
            var: str = line.get("input", {}).get("var", "")
            data[var] = input()

        elif key == "if":
            condition: bool = run({"code": line.get("if", {}).get("condition", [])})
            if condition:
                run({"parameters": {}, "code": line.get("if", {}).get("then", [])})
            else:
                run({"parameters": {}, "code": line.get("if", {}).get("else", [])})

        elif key == "return":
            if "var" in line.get("return", {}):
                return data[line.get("return", {}).get("var", "")]
            elif "const" in line.get("return", {}):
                return line.get("return", {}).get("const", "")
            else:
                return return_value

        elif key == "equal":
            value1: Any = run({"code": [line.get("equal", [])[0]]})
            value2: Any = run({"code": [line.get("equal", [])[1]]})
            return value1 == value2

        elif key == "greater":
            value1: Any = run({"code": [line.get("greater", [])[0]]})
            value2: Any = run({"code": [line.get("greater", [])[1]]})
            return value1 > value2
            
        elif key == "var":
            return data[line.get("var", "")]

        elif key == "const":
            return line.get("const", "")

        elif key == "random":
            m1: int = line.get("random", {}).get("min", 0)
            m2: int = line.get("random", {}).get("max", 0)
            return random.randint(m1, m2)
        
        elif key == "int":
            return int(run({"code": line.get("int", [])}))

        elif key == "end":
            exit()

        elif key in functions:
            run(functions.get(key, {}))

        exec_line += 1

run(functions["main"])
