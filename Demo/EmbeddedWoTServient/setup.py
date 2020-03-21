from setuptools import setup, find_packages

setup(
    name='embeddedWotServient',
    version='0.1',
    py_modules=['embeddedWotServient'],
    install_requires=['Click',],
    entry_points='''
        [console_scripts]
        embeddedWotServient=embeddedWotServient:cli
    '''
)