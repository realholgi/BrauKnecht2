import os


existing = os.environ.get("PYTHONWARNINGS")
syntax_filter = "ignore::SyntaxWarning"

if existing:
    if syntax_filter not in existing.split(","):
        os.environ["PYTHONWARNINGS"] = f"{existing},{syntax_filter}"
else:
    os.environ["PYTHONWARNINGS"] = syntax_filter
