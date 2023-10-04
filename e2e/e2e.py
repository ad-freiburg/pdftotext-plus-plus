#!/usr/bin/python3

import argparse
import sys

from os.path import isabs
from pathlib import Path

from e2e_analyzer import E2eAnalyzer
from e2e_runner import E2eRunner
from e2e_utils import sprintln, Style

# ==================================================================================================

DESCRIPTION: str = "A tool for running & analyzing the E2E tests of pdftotext++."

USAGE: str = """e2e.py <command> [<options>]

where <command> is one of:

  run        runs the E2E tests of pdftotext++
  analyze    analyzes E2E test reports produced during previous test runs

Type \"e2e.py <command> --help\" to read about the options of a specific subcommand.
"""

# ==================================================================================================

# The default path to the pdftotext++ executable.
DEFAULT_PPP_PATH: Path = Path("/usr/bin/pdftotext++")

# The default path to the config file (e.g., specifying the tests to run).
DEFAULT_CONFIG_FILE_PATH: Path = Path(__file__).parent / "config.yml"

# The default path to the directory to scan for report files.
DEFAULT_REPORT_DIR_PATH: str = E2eAnalyzer.LATEST_TEST_RESULT_DIR

# The default file name mask to be used to match report files.
DEFAULT_REPORT_FILE_NAME_MASK: Path = "*.report"

# ==================================================================================================


class E2ECommandLineInterface:
    """
    A class for reading and processing the command line arguments of the different subcommands
    available for running and analyzing the E2E tests of pdftotext++.
    """

    def __init__(self) -> None:
        """
        Reads and processes the specified subcommand from the command line and invokes the method
        which is responsible for reading and processing the arguments of that subcommand.
        """

        parser = argparse.ArgumentParser(
            formatter_class=argparse.ArgumentDefaultsHelpFormatter,
            description=f"{Style.BOLD}{DESCRIPTION}{Style.NONE}",
            usage=USAGE
        )

        # Add an argument for specifying the subcommand to run.
        parser.add_argument("command", help="the command to run")

        args = parser.parse_args(sys.argv[1:2])

        if not hasattr(self, args.command):
            sprintln(f"unrecognized command \"{args.command}\"", Style.ERROR)
            sprintln("type \"e2e.py --help\" to get usage info.")
            exit(1)

        getattr(self, args.command)()

    # ==============================================================================================

    def run(self) -> None:
        """
        Reads and processes the arguments of the subcommand "run".
        """

        parser = argparse.ArgumentParser(
            formatter_class=argparse.ArgumentDefaultsHelpFormatter,
            description=f"{Style.BOLD}Runs the E2E tests of pdftotext++.{Style.NONE}",
            usage="e2e.py run [<options>]"
        )

        # Add an argument for specifying the path to the pdftotext++ executable.
        parser.add_argument(
            "--ppp",
            metavar="<path>",
            required=False,
            default=DEFAULT_PPP_PATH,
            type=lambda p: Path(p) if isabs(p) else parser.error(f"path '{p}' must be absolute"),
            help="set the absolute path to the pdftotext++ executable"
        )

        # Add an argument for specifying the path to the config file (specifying the E2E tests).
        parser.add_argument(
            "--config",
            metavar="<path>",
            required=False,
            default=DEFAULT_CONFIG_FILE_PATH,
            type=lambda p: Path(p) if isabs(p) else parser.error(f"path '{p}' must be absolute"),
            help="set the absolute path to the config file in which the tests to run are specified"
        )

        args = parser.parse_args(sys.argv[2:])

        E2eRunner(ppp_path=args.ppp, config_file_path=args.config).run()

    def analyze(self) -> None:
        """
        Reads and processes the arguments of the subcommand "analyze".
        """

        parser = argparse.ArgumentParser(
            formatter_class=argparse.ArgumentDefaultsHelpFormatter,
            description=f"{Style.BOLD}"
                        "Analyzes E2E test reports produced during previous test runs."
                        f"{Style.NONE}",
            usage="e2e.py analyze [<options>]"
        )

        # Add an argument for specifying the directory to scan for report files.
        parser.add_argument(
            "--dir",
            metavar="<str>",
            required=False,
            default=DEFAULT_REPORT_DIR_PATH,
            help="set the path to the directory to scan for report files; use the keyword "
                 f"'{E2eAnalyzer.LATEST_TEST_RESULT_DIR}' to select the directory containing the "
                 "report files produced during the *latest* test run."
        )

        # Add an argument for specifying the file name mask to be used to match the report files.
        parser.add_argument(
            "--mask",
            metavar="<str>",
            required=False,
            default=DEFAULT_REPORT_FILE_NAME_MASK,
            help="set the file name mask to be used to match the report files to analyze"
        )

        # Add an argument for selecting the "VS code" mode.
        parser.add_argument(
            "--vscode",
            action="store_true",
            help="select a mode that opens the expected and actual output of each failed test case "
                 "in Visual Studio Code side-by-side, with the differences highlighted in color"
        )

        args = parser.parse_args(sys.argv[2:])

        E2eAnalyzer(
            dir_path=args.dir,
            report_file_name_mask=args.mask,
            vscode=args.vscode
        ).run()

# ==================================================================================================


if __name__ == "__main__":
    E2ECommandLineInterface()
