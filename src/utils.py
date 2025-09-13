from typing import NoReturn
from pathlib import Path
import subprocess
import platform
import shutil
import shlex
import sys
import os
import re

def exit(code: int = 0) -> NoReturn:
    sys.exit(code)

def error(desc: str, code: int = 1) -> NoReturn:
    print(f'ERROR: {desc}')
    exit(code)

def warning(desc: str) -> None:
    print(f'WARNING: {desc}')

def escape_shell_string_if_needed(str : str) -> str:
    escape = False
    for ch in str:
        if ch.isspace():
            escape = True
    if escape:
        str = '"' + str + '"'
    return str

def to_shell_string(cmd : list[str]) -> str:
    shell_string: str = ''
    for i, arg in enumerate(cmd):
        shell_string += escape_shell_string_if_needed(arg)
        if i < len(cmd) - 1:
            shell_string += ' '
    return shell_string

def run(cmd : str | list[str], capture : bool = False, shell : bool = False, cwd : Path = Path.cwd()) -> int | str:
    if shell and (type(cmd) is list):
        final_cmd = to_shell_string(cmd)
    else:
        final_cmd = cmd
        
    #if shell:
    #    print(f'$ {final_cmd}')
        
    output = subprocess.run(final_cmd, cwd=cwd, capture_output=capture, text=capture, shell=shell)
    
    if capture:
        return output.stdout
    else:
        return output.returncode
    
def copy_dir(src : Path, dest : Path, exist_ok : bool = True) -> None:
    shutil.copytree(src, dest, dirs_exist_ok=exist_ok)
    
def copy_file(src : Path, dest : Path, exist_ok : bool = True) -> None:
    if not exist_ok:
        if dest.exists():
            error(f"File {dest} already exists!")
    shutil.copy(src, dest)
    
def copy(src : Path, dest : Path, exist_ok : bool = True) -> None:
    if src.is_file():
        copy_file(src, dest, exist_ok)
    else:
        copy_dir(src, dest, exist_ok)
        
def mkdir(path : Path, parents : bool = True, exist_ok : bool = True) -> None:
    path.mkdir(parents=parents, exist_ok=exist_ok)
    
def list_subdirs(path : Path) -> list[str]:
    return os.listdir(path)

def string_subdirs(path : Path) -> str:
    return ', '.join(list_subdirs(path))
       
os_str     : str  = platform.system()
os_posix   : bool = not ('windows' in os_str.lower())
os_windows : bool = ('windows'     in os_str.lower())
os_linux   : bool = ('linux'       in os_str.lower())
os_darwin  : bool = ('darwin'      in os_str.lower())

arch_str   : str = platform.architecture()[0]
arch_match       = re.match(r'(.*)bit', arch_str, re.IGNORECASE)
if arch_match:
    arch_str = 'x' + arch_match.group(1)

os_triplet : str  = os_str.lower() + '-' + arch_str.lower()

if os_windows:
    os_file_extension : dict[str,str] = {'executable':'.exe', 'static':'.a', 'dynamic':'.dll'}
    os_file_prefix    : dict[str,str] = {'executable':'', 'static':'lib', 'dynamic':''}
else:
    os_file_extension : dict[str,str] = {'executable':'', 'static':'.a', 'dynamic':'.so'}
    os_file_prefix    : dict[str,str] = {'executable':'', 'static':'lib', 'dynamic':''}

if getattr(sys, 'frozen', False):
    buildit_dir : Path = Path(os.path.dirname(sys.executable))
else:
    buildit_dir : Path = Path(os.path.dirname(__file__))
    
config_name = 'buildme.yaml'
    
templates_dir : Path = buildit_dir / Path('project_templates')
if not templates_dir.exists():
    templates_dir = buildit_dir.parent / Path('project_templates')
if not templates_dir.exists():
    error('Couldnt find the templates directory make sure it is in the directory where the buildit executable is or in its parent!')
    
pkgconf_available : bool = False
pkgconf_path      : str  = 'pkgconf'
if not shutil.which(pkgconf_path):
    pkgconf_path = 'pkg-config'
if not shutil.which(pkgconf_path):
    warning('PKGCONF_LIBS DISABLED! Couldnt find the pkgconfig tool make sure you have installed pkgconf or pkg-config in your PATH!')
else:
    pkgconf_available = True
     
# KINDA CONFIG
    
compiler_library_flag      : str = '-l'
compiler_library_dir_flag  : str = '-L'
compiler_include_dir_flag  : str = '-I'
compiler_compile_only_flag : str = '-c'
compiler_output_name_flag  : str = '-o'

linker_shared_lib_flag     : str = '-shared'

# END OF CONFIG
            
def parse_pkgconf(pkgconf_out : str) -> dict:
    parsed_args = {'libs':[],'lib_dirs':[],'include_dirs':[],'flags':[]}
    
    for flag in pkgconf_out.split():
        flag = flag.strip()
        if flag.startswith('-l'):
            parsed_args['libs'].append(flag.removeprefix('-l'))
        elif flag.startswith('-L'):
            parsed_args['lib_dirs'].append(flag.removeprefix('-L'))
        elif flag.startswith('-I'):
            parsed_args['include_dirs'].append(flag.removeprefix('-I'))
        else:
            parsed_args['flags'].append(flag)
    
    return parsed_args