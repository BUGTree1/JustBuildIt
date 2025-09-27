from pathlib import Path
import config_parser
import build_utils
import arg_parser
import builder
import utils

def main():
    parsed_args = arg_parser.parse()
    parsed_config = config_parser.parse(parsed_args['project_dir'])
    if utils.pkgconf_available:
        parsed_config = utils.run_pkgconf(parsed_config, parsed_args['project_dir'])
    build_utils.exec(parsed_args, parsed_config)
    builder.build(parsed_config, parsed_args)
    
if __name__ == "__main__":
    main()