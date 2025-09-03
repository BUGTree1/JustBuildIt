from typing import NoReturn
from pathlib import Path
import subprocess
import platform
import shutil
import sys
import os

def exit(code: int = 0) -> NoReturn:
    sys.exit(code)

def error(desc: str, code: int = 1) -> NoReturn:
    print(f'ERROR: {desc}')
    exit(code)

def warning(desc: str) -> None:
    print(f'WARNING: {desc}')

def run(cmd : list[str], cwd : Path = Path.cwd()) -> int:
    print(f'$ {cmd}')
    return subprocess.run(cmd,cwd=cwd).returncode

def run_capture(cmd : list[str], cwd : Path = Path.cwd()) -> str:
    print(f'$ {cmd}')
    process = subprocess.run(cmd,cwd=cwd, capture_output=True, text=True)
    if process.returncode != 0:
        error(f'Command: {cmd} exited with code {process.returncode}! and outputted {process.stdout}')
    return process.stdout

def run_shell(cmd : list[str], cwd : Path = Path.cwd()) -> int:
    print(f'$ {cmd}')
    return subprocess.run(cmd,cwd=cwd,shell=True).returncode

def run_capture_shell(cmd : list[str], cwd : Path = Path.cwd()) -> str:
    print(f'$ {cmd}')
    process = subprocess.run(cmd,cwd=cwd, capture_output=True, text=True ,shell=True)
    if process.returncode != 0:
        error(f'Command: {cmd} exited with code {process.returncode}! and outputted {process.stdout}')
    return process.stdout
    
def copy_tree(src : Path, dest : Path, exist_ok : bool = True) -> None:
    shutil.copytree(src, dest, dirs_exist_ok=exist_ok)
    
def list_subdirs(path : Path) -> list[str]:
    return os.listdir(path)

def string_subdirs(path : Path) -> str:
    return ', '.join(list_subdirs(path))
     
# KINDA CONFIG
       
os_str     : str  = platform.system()
os_posix   : bool = not ('windows' in os_str.lower())
os_windows : bool = ('windows' in os_str.lower())
os_linux   : bool = ('linux' in os_str.lower())

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
    
compiler_library_flag      : str = '-l'
compiler_library_dir_flag  : str = '-L'
compiler_include_dir_flag  : str = '-I'
compiler_compile_only_flag : str = '-c'
compiler_output_name_flag  : str = '-o'

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