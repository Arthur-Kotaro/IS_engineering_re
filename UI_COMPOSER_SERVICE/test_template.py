import yaml
from src.models import Template, SourceConfig

def load_template(name):
    path = f"templates/{name}.yaml"
    print(f"Loading: {path}")
    
    with open(path, 'r') as f:
        data = yaml.safe_load(f)
    
    print(f"Data keys: {list(data.keys())}")
    print(f"Sources: {data.get('sources', [])}")
    
    sources = []
    for s in data.get('sources', []):
        print(f"  Source: {s}")
        sources.append(SourceConfig(**s))
    
    template = Template(
        name=data['name'],
        endpoint=data['endpoint'],
        title=data['title'],
        description=data.get('description'),
        sources=sources,
        ui=data.get('ui', {})
    )
    
    print(f"Template loaded: {template.name}")
    print(f"  Sources: {len(template.sources)}")
    return template

if __name__ == "__main__":
    try:
        t = load_template("hr_search_user")
        print("SUCCESS")
    except Exception as e:
        print(f"ERROR: {e}")
