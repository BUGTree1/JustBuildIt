from pathlib import Path
import config_parser
import arg_parser
import builder

def main():
    parsed_args = arg_parser.parse()
    parsed_config = config_parser.parse(parsed_args['project_dir'])
    builder.build(parsed_config,parsed_args)
    
if __name__ == "__main__":
    main()