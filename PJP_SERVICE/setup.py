"""Setup script for PJP Service with Python 3.14 compatibility"""
from setuptools import setup, find_packages

setup(
    name="pjp-service",
    version="1.0.0",
    packages=find_packages(),
    install_requires=[
        "fastapi>=0.115.0",
        "uvicorn[standard]>=0.34.0",
        "sqlalchemy>=2.0.36",
        "asyncpg>=0.30.0",
        "pydantic>=2.10.0",
        "pydantic-settings>=2.6.0",
        "python-dotenv>=1.0.0",
        "httpx>=0.27.0",
        "python-multipart>=0.0.20",
    ],
    python_requires=">=3.12",
)
