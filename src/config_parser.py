import yaml
from pathlib import Path

def parse(project_dir : Path) -> dict:
    config = {}
    with open(project_dir / "buildme.yaml") as stream:
        try:
            config = yaml.safe_load(stream)
        except yaml.YAMLError as exc:
            print(exc)
    return config