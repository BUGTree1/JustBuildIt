from pathlib import Path
import subprocess
import platform
import shutil
import sys
import os

def exit(code = 0):
    sys.exit(code)

def error(desc, code = 1):
    print(f'ERROR: {desc}')
    exit(code)

def warning(desc):
    print(f'WARNING: {desc}')

def run(cmd, cwd=Path.cwd()):
    return subprocess.run(cmd,cwd=cwd).returncode
    
def copy_tree(src, dest, exist_ok=True):
    shutil.copytree(src, dest, dirs_exist_ok=exist_ok)
    
def list_subdirs(path):
    return os.listdir(path)

def string_subdirs(path):
    return ', '.join(list_subdirs(path))
            
os_str = platform.system()
os_posix = not ('windows' in os_str.lower())
os_windows = ('windows' in os_str.lower())
os_linux = ('linux' in os_str.lower())

if getattr(sys, 'frozen', False):
    buildit_dir = Path(os.path.dirname(sys.executable))
else:
    buildit_dir = Path(os.path.dirname(__file__))
    
config_name = 'buildme.yaml'
    
templates_dir = buildit_dir / Path('project_templates')
if not templates_dir.exists():
    templates_dir = buildit_dir.parent / Path('project_templates')
if not templates_dir.exists():
    error('Couldnt find the templates directory make sure it is in the directory where the buildit executable is or in its parent!')