from pathlib import Path
import utils
import yaml

def parse(project_dir : Path) -> dict:
    config : dict[str,str] = {}
    config_path = project_dir / utils.config_name
    
    if not config_path.exists():
        utils.error('Project config not found!')
        
    with open(config_path) as config_stream:
        try:
            config = yaml.safe_load(config_stream)
        except yaml.YAMLError as exc:
            print("CONFIG YAML ERROR:")
            print(exc)
            utils.exit(1)
            
    final_config = {}
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
            
    return final_config