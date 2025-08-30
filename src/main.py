import config_parser
from pathlib import Path

def main():
    config = config_parser.parse(Path('project_templates') / 'cpp')
    print(config)
    
if __name__ == "__main__":
    main()