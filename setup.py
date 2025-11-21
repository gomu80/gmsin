"""
Setup script for Structured Light Scanner
"""

from setuptools import setup, find_packages
import os

# Read README
with open('README.md', 'r', encoding='utf-8') as f:
    long_description = f.read()

# Read requirements
with open('requirements.txt', 'r', encoding='utf-8') as f:
    requirements = [line.strip() for line in f if line.strip() and not line.startswith('#')]

setup(
    name='structured-light-scanner',
    version='1.0.0',
    author='GMSIN Team',
    description='Structured Light 3D Scanning System',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/gomu80/gmsin',
    package_dir={'': 'src'},
    packages=find_packages(where='src'),
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Science/Research',
        'Topic :: Scientific/Engineering :: Image Recognition',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
    ],
    python_requires='>=3.8',
    install_requires=requirements,
    extras_require={
        'dev': [
            'pytest>=7.0',
            'pytest-cov>=3.0',
            'black>=22.0',
            'flake8>=4.0',
            'mypy>=0.950',
        ],
        'docs': [
            'sphinx>=4.0',
            'sphinx-rtd-theme>=1.0',
        ],
    },
    entry_points={
        'console_scripts': [
            'structured-light-scan=structured_light.scanner:main',
        ],
    },
    include_package_data=True,
    package_data={
        'structured_light': ['config/*.yaml'],
    },
)
