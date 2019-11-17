import click

@click.group(invoke_without_command=True)

@click.pass_context
def cli(ctx):
    """WoT module for build TDs and executable scripts for embedded systems"""
    if ctx.invoked_subcommand is None:
        click.echo('This module allow you to build custom Thing Descriptions and executable scripts for expose Thing on embedded systems')
        click.echo('Use --help option to see documentation')
        start()
    else:
        pass


@click.command()

@click.confirmation_option(prompt='\nWould you use wizard?')
def start():
    click.echo('Wizard start...')

if __name__ == "__main__":
   cli()  
