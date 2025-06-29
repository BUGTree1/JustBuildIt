from pathlib import Path
from sys import exit
import subprocess
import argparse
import platform
import shutil
import runpy
import time
import sys
import os

DEBUG = False

def error(desc: str, err_code: int = 1):
    print("Error: " + desc)
    exit(err_code)

def print_args(args : list[str]):
    print('$ ',end='')
    for i in range(0,len(args)):
        if not DEBUG and Path(args[i]).is_absolute():
            print(Path(args[i]).name,end='')
        else:
            print(args[i],end='')
        if i < len(args) - 1:
            print(' ',end='')
    print('')

platform_windows : bool = platform.system().lower() == "windows"
platform_macos : bool = platform.system().lower() == "macos"
platform_linux : bool = platform.system().lower() == "linux"
platform_other : bool = (not platform_windows) and (not platform_macos) and (not platform_linux)

platform_x64 : bool = platform.architecture()[0] == "64bit"
platform_x32 : bool = platform.architecture()[0] == "32bit"
platform_arm : bool = "arm" in platform.uname()[4].lower()

triplet_arch : str = ("arm" if platform_arm else "x") + platform.architecture()[0].removesuffix('bit')
target_triplet : str = platform.system().lower() + '-' + triplet_arch

# frozen attribute is when script is in exe by pyinstaller
if getattr(sys, 'frozen', False):
    buildit_path : Path = Path(sys.executable).parent.resolve()
else:
    buildit_path : Path = Path(__file__).parent.resolve()

template_path : Path = buildit_path / 'cpp_project_templates'

desc = """Simple python 3.8+ build system for c++ that: 
Is designed for windows MinGW and Linux (But can adapt to many other toolchains) , 
Works on projects  , 
Can build them with one command in the terminal , 
Rebuilds itself when reconfigured , 
By default is itself built by GNU Make , """

epi = "Currently there are no installed templates! install default with -i flag"
template_names = []
if template_path.exists() and len(next(os.walk(template_path))[1]) > 0:
    epi = "Available templates are: "
    for template_name in next(os.walk(template_path))[1]:
        template_names.append(template_name)
        
    epi += ' , '.join(template_names)


parser = argparse.ArgumentParser(prog='buildit',description=desc,epilog=epi)
parser.add_argument('project_path',nargs='?',default='.',help='path to the project')           # positional argument
parser.add_argument('-t', '--template',metavar='TEMPLATE_NAME',type=str, help='generate template')      # option that takes a value
parser.add_argument('-c', '--clean', action='store_true', help='clean any build files')
parser.add_argument('-r', '--rebuild', action='store_true', help='rebuild whole project')
parser.add_argument('-i', '--init', action='store_true', help='init a git repo')
parser.add_argument('-p', '--push',metavar='COMMIT_NAME',type=str, help='push to remote repo with commit name')
parser.add_argument('-a', '--add-remote',metavar='REMOTE_URL',type=str, help='add remote URL')
args = parser.parse_args()

project_path : Path = Path.cwd() if (args.project_path == None or args.project_path == '.') else Path(args.project_path)

if not project_path.is_absolute():
    project_path = Path.cwd() / project_path
    project_path = project_path.resolve()
    pass

if args.init:
    subprocess.run(['git','init','-b','main'],cwd=project_path)
    exit(0)

if args.add_remote != None:
    subprocess.run(['git','remote','add','origin',args.add_remote],cwd=project_path)
    exit(0)

if args.push != None:
    subprocess.run(['git','add','.'],cwd=project_path)
    subprocess.run(['git','commit','-m',args.push],cwd=project_path)
    subprocess.run(['git','push','origin','main'],cwd=project_path)
    exit(0)

if args.template != None:
    print('generating template from: ' + str(template_path / args.template))
    shutil.copytree(template_path / args.template, project_path,dirs_exist_ok=True)
    pass

create_vscode_settings = False
run_after_build        = False
proj_name              = "NAME_NOT_SET"
proj_version           = "0.0.0.0"
file_name              = "relative_NOT_SET"
compiler               = "g++"
c_compiler             = ""
cxx_compiler           = ""
linker                 = "g++"
flags                  = []
compiler_flags         = []
linker_flags           = []
output_path            = "OUTPUT_PATH_NOT_SET"
source_path            = "SOURCE_PATH_NOT_SET"
obj_dir                = "obj"
libs                   = []
lib_paths              = []
include_paths          = []
output_name_arg        = "-o"
compile_only_arg       = "-c"
library_arg            = "-l"
library_path_arg       = "-L"
include_path_arg       = "-I"

if not (project_path / 'buildme.py').exists():
    error("Not a valid project: Does not have an buildme.py")

print('reading config from: ' + str(project_path / 'buildme.py'))

# I have no other ideas than this "hack" for running the script and
# retaining the variables because exec() does not work in functions
# OBVIOUSLY ( another python classic XDDD )
config_globals = globals()
config_globals.update(locals())
config_vars = runpy.run_path(str(project_path / 'buildme.py'),config_globals ,"__main__")

create_vscode_settings = config_vars['create_vscode_settings']
run_after_build        = config_vars['run_after_build']
proj_name              = config_vars['proj_name']
proj_version           = config_vars['proj_version']
file_name              = config_vars['file_name']
compiler               = config_vars['compiler']
c_compiler             = config_vars['c_compiler']
cxx_compiler           = config_vars['cxx_compiler']
linker                 = config_vars['linker']
flags                  = config_vars['flags']
compiler_flags         = config_vars['compiler_flags']
linker_flags           = config_vars['linker_flags']
output_path            = config_vars['output_path']
source_path            = config_vars['source_path']
obj_dir                = config_vars['obj_dir']
libs                   = config_vars['libs']
lib_paths              = config_vars['lib_paths']
include_paths          = config_vars['include_paths']
output_name_arg        = config_vars['output_name_arg']
compile_only_arg       = config_vars['compile_only_arg']
library_arg            = config_vars['library_arg']
library_path_arg       = config_vars['library_path_arg']
include_path_arg       = config_vars['include_path_arg']

build_dir = project_path / output_path / target_triplet
if platform_windows:
    file_name = str(Path(file_name).with_suffix('.exe'))

if cxx_compiler == "":
    cxx_compiler = compiler
    
if c_compiler == "":
    c_compiler = compiler
    
if compiler == "":
    compiler = c_compiler if c_compiler != "" else cxx_compiler
    
if create_vscode_settings:
    if not (project_path / ".vscode").exists():
        (project_path / ".vscode").mkdir(parents=True)

    vscode_config = open(project_path / ".vscode" / "c_cpp_properties.json","w+")
    vscode_config.write("{\n\"configurations\": [\n{\n\"name\": \"Generic\",\n\"includePath\": [")

    for i in range(0,len(include_paths)):
        vscode_config.write("\"" + include_paths[i] + "\"")
        if i < len(include_paths) - 1:
            vscode_config.write(',')

    vscode_config.write("],\n\"defines\": [\n")
    vscode_config.write("\n],\n\"compilerPath\": ")
    c_compiler_abs_path : str = str(shutil.which(c_compiler))
    c_compiler_abs_path = c_compiler_abs_path.replace('\\','\\\\')
    vscode_config.write("\"" + c_compiler_abs_path + "\"")
    vscode_config.write(",\n\"cStandard\": \"c17\",\n\"cppStandard\": \"gnu++17\",\n\"intelliSenseMode\": ")
    if platform_windows:
        if compiler == 'clang' or compiler == 'clang++':
            vscode_config.write("\"windows-clang-" + triplet_arch + "\"")
        elif compiler == 'msvc' or compiler == 'cl':
            vscode_config.write("\"windows-msvc-" + triplet_arch + "\"")
        else:
            vscode_config.write("\"windows-gcc-" + triplet_arch + "\"")
    elif platform_macos:
        if compiler == 'clang' or compiler == 'clang++':
            vscode_config.write("\"macos-clang-" + triplet_arch + "\"")
        elif compiler == 'msvc' or compiler == 'cl':
            vscode_config.write("\"macos-msvc-" + triplet_arch + "\"")
        else:
            vscode_config.write("\"macos-gcc-" + triplet_arch + "\"")
    elif platform_linux:
        if compiler == 'clang' or compiler == 'clang++':
            vscode_config.write("\"linux-clang-" + triplet_arch + "\"")
        elif compiler == 'msvc' or compiler == 'cl':
            vscode_config.write("\"linux-msvc-" + triplet_arch + "\"")
        else:
            vscode_config.write("\"linux-gcc-" + triplet_arch + "\"")
    else:
        if compiler == 'clang' or compiler == 'clang++':
            vscode_config.write("\"clang-" + triplet_arch + "\"")
        elif compiler == 'msvc' or compiler == 'cl':
            vscode_config.write("\"msvc-" + triplet_arch + "\"")
        else:
            vscode_config.write("\"gcc-" + triplet_arch + "\"")
        
    vscode_config.write("\n}]\n,\n\"version\": 4\n}")

    vscode_config.close()

if args.clean:
    print('cleaning any build files in' + str(build_dir))
    shutil.rmtree(build_dir)
    args.rebuild = True
    pass

last_build_time : float = 0.0
if build_dir.exists():
    if (build_dir / file_name).exists():
        last_build_time = os.path.getmtime(build_dir / file_name)
        print('last build was on: ' + time.ctime(last_build_time))
        if os.path.getmtime(project_path / 'buildme.py') > last_build_time:
            args.rebuild = True
    else:
        args.rebuild = True
else:
    args.rebuild = True
    build_dir.mkdir(parents=True)
    
if not (build_dir / obj_dir).exists():
    (build_dir / obj_dir).mkdir(parents=True)

has_to_compile = args.rebuild

o_files : list[Path] = []
cpp_files : list[Path] = []
cpp_files_to_compile : list[tuple[Path,Path]] = []  
for cpp_file in (project_path / source_path).rglob("**.cpp"):
    cpp_relative = cpp_file.relative_to(project_path / source_path)
    o_relative = Path(cpp_file.with_suffix('.o').name)
    cpp_files.append(cpp_relative)
    o_files.append(o_relative)
    
    if args.rebuild:
        cpp_files_to_compile.append((cpp_relative,o_relative))
    else:
        if os.path.getmtime(cpp_file) > last_build_time:
            cpp_files_to_compile.append((cpp_relative,o_relative))
            has_to_compile = True
            pass
        pass
    pass
    
c_files : list[Path] = []
c_files_to_compile : list[tuple[Path,Path]] = []  
for c_file in (project_path / source_path).rglob("**.c"):
    c_relative = c_file.relative_to(project_path / source_path)
    o_relative = Path(c_file.with_suffix('.o').name)
    c_files.append(c_relative)
    o_files.append(o_relative)
    
    if args.rebuild:
        c_files_to_compile.append((c_relative,o_relative))
    else:
        if os.path.getmtime(c_file) > last_build_time:
            c_files_to_compile.append((c_relative,o_relative))
            has_to_compile = True
            pass
        pass
    pass

has_errors = False
if has_to_compile:
    for c_file,o_file in c_files_to_compile:
        compiler_args = []
        compiler_args.append(c_compiler)
        compiler_args.append(compile_only_arg)
        compiler_args.append(str(project_path / source_path / c_file))
        compiler_args.append(output_name_arg)
        compiler_args.append(str(build_dir / obj_dir / o_file))
        compiler_args.extend(compiler_flags)
        compiler_args.extend(flags)
        for include_path in include_paths:
            compiler_args.append(include_path_arg)
            compiler_args.append(include_path)
        print_args(compiler_args)
        status = subprocess.run(compiler_args,cwd=project_path).returncode
        if status != 0:
            has_errors = True
    
    for cpp_file,o_file in cpp_files_to_compile:
        compiler_args = []
        compiler_args.append(cxx_compiler)
        compiler_args.append(compile_only_arg)
        compiler_args.append(str(project_path / source_path / cpp_file))
        compiler_args.append(output_name_arg)
        compiler_args.append(str(build_dir / obj_dir / o_file))
        compiler_args.extend(compiler_flags)
        compiler_args.extend(flags)
        for include_path in include_paths:
            compiler_args.append(include_path_arg)
            compiler_args.append(include_path)
        print_args(compiler_args)
        status = subprocess.run(compiler_args,cwd=project_path).returncode
        if status != 0:
            has_errors = True
        
if not has_errors and has_to_compile:
    linker_args = []
    linker_args.append(linker)
    for o_file in o_files:
        linker_args.append(str(build_dir / obj_dir / o_file))
    
    linker_args.append(output_name_arg)
    linker_args.append(str(build_dir / file_name))
    linker_args.extend(flags)
    linker_args.extend(linker_flags)
    for include_path in include_paths:
        linker_args.append(include_path_arg)
        linker_args.append(include_path)
    
    for lib_path in lib_paths:
        linker_args.append(library_path_arg)
        linker_args.append(lib_path)
    
    for lib in libs:
        linker_args.append(library_arg)
        linker_args.append(lib)
    
    print_args(linker_args)
    status = subprocess.run(linker_args,cwd=project_path).returncode
    if status != 0:
        has_errors = True
        
if run_after_build:
    if build_dir.exists():
        if (build_dir / file_name).exists():
            subprocess.run((build_dir / file_name),cwd=build_dir)