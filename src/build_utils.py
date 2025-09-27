from pathlib import Path, WindowsPath
import shutil
import utils

def exec(args: dict, config: dict) -> None:
    if config["create_vscode_settings"]:
        include_dirs = config["include_dirs"]
        compiler = config["compiler"]
        
        settings_path = Path(args["project_dir"]) / '.vscode' / 'c_cpp_properties.json'
        utils.mkdir(settings_path.parent)
        with open(settings_path,mode = 'w') as file:
            file.write("{\n\"configurations\": [\n{\n\"name\": \"Generic\",\n\"includePath\": [\n")
            
            file.write(', '.join(list(map(lambda dir: "\"" + str(dir).replace('\\','\\\\') + "\"", include_dirs))))

            file.write("\n],\n\"defines\": [\n")
            file.write("\n],\n\"compilerPath\": ")
            compiler_abs_path : str = str(shutil.which(compiler)).replace('\\','\\\\')
            file.write("\"" + compiler_abs_path + "\"")
            file.write(",\n\"cStandard\": \"c17\",\n\"cppStandard\": \"gnu++17\",\n\"intelliSenseMode\": ")
            file.write("\"")
            if utils.os_windows or utils.os_darwin or utils.os_linux:
                if utils.os_darwin:
                    file.write("macos-")
                else:
                    file.write(utils.os_str.lower() + "-")
                
                if compiler == 'clang' or compiler == 'clang++':
                    file.write("clang")
                elif compiler == 'cl':
                    file.write("msvc")
                else:
                    file.write("gcc")
            else:
                if compiler == 'clang' or compiler == 'clang++':
                    file.write("clang")
                elif compiler == 'cl':
                    file.write("msvc")
                else:
                    file.write("gcc")
            file.write("-" + utils.arch_str + "\"")
            file.write("\n}]\n,\n\"version\": 4\n}")