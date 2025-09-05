from pathlib import Path
import utils
import yaml

def parse(project_dir : Path) -> dict:
    config      : dict[str,str] = {}
    config_path : Path = project_dir / utils.config_name
    
    if not config_path.exists():
        utils.error('Project config not found!')
        
    with open(config_path) as config_stream:
        try:
            config = yaml.safe_load(config_stream)
        except yaml.YAMLError as exc:
            print("CONFIG YAML ERROR:")
            print(exc)
            utils.exit(1)
            
    final_config    = {}
    platform_config = {}
    for key,val in config.items():
        if utils.os_windows and (key.lower() == 'windows' or key.lower() == 'win'):
            platform_config = val
        elif utils.os_posix and (key.lower() == 'posix'):
            platform_config = val
        elif utils.os_linux and (key.lower() == 'linux'):
            platform_config = val
        else:
            final_config[key] = val
        
    if platform_config != {}:
        for key,val in platform_config.items(): #type: ignore
            final_config[key] = val
            
    if not 'create_vscode_settings' in final_config:
        final_config['create_vscode_settings'] = False
    if not 'run_after_build' in final_config:
        final_config['run_after_build'] = False
    if not 'auto_out_file_ext' in final_config:
        final_config['auto_out_file_ext'] = True
    if not 'build_type' in final_config:
        final_config['build_type'] = 'executable'
    if not 'proj_name' in final_config:
        final_config['proj_name'] = 'PROJ_NAME_NOT_SET'
    if not 'proj_version' in final_config:
        final_config['proj_version'] = '0.0.0.0'
    if not 'file_name' in final_config:
        final_config['file_name'] = 'FILENAME_NOT_SET'
    if not 'compiler' in final_config:
        final_config['compiler'] = 'COMPILER_NOT_SET'
    if not 'c_compiler' in final_config:
        final_config['c_compiler'] = ''
    if not 'cxx_compiler' in final_config:
        final_config['cxx_compiler'] = ''
    if not 'linker' in final_config:
        final_config['linker'] = 'LINKER_NOT_SET'
    if not 'flags' in final_config:
        final_config['flags'] = []
    if not 'compiler_flags' in final_config:
        final_config['compiler_flags'] = []
    if not 'linker_flags' in final_config:
        final_config['linker_flags'] = []
    if not 'output_dir' in final_config:
        final_config['output_dir'] = ''
    if not 'object_dir' in final_config:
        final_config['object_dir'] = 'obj'
    if not 'source_dir' in final_config:
        final_config['source_dir'] = ''
    if not 'libs' in final_config:
        final_config['libs'] = []
    if not 'pkgconf_libs' in final_config:
        final_config['pkgconf_libs'] = []
    if not 'lib_dirs' in final_config:
        final_config['lib_dirs'] = []
    if not 'include_dirs' in final_config:
        final_config['include_dirs'] = []
    if not 'run_args' in final_config:
        final_config['run_args'] = []
    if not 'exec_postbuild' in final_config:
        final_config['exec_postbuild'] = []
    if not 'exec_prebuild' in final_config:
        final_config['exec_prebuild'] = []
            
    return final_config