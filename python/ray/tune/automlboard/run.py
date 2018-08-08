from django.core.management import execute_from_command_line
from common.exception import DatabaseError

import argparse
import django
import os
import re


def run_board(args):
    """
    Main entry for AutoMLBoard.

    Args:
        args: args parsed from command line
    """
    init_config(args)

    # backend service, should import after django settings initialized
    from backend.collector import CollectorService

    service = CollectorService(
        args.logdir,
        args.reload_interval,
        share_mode=args.share_mode,
        standalone=False,
        log_level=args.log_level)
    service.run()

    # frontend service
    print("try to start automlboard on port %s\n" % args.port)
    command = [
        'manage.py', 'runserver',
        '0.0.0.0:%s' % args.port, '--noreload'
    ]
    execute_from_command_line(command)


def init_config(args):
    """
    Initialize configs of the service.

    Do the following things:
    1. automl board settings
    2. database settings
    3. django settings
    """
    os.environ["AUTOMLBOARD_LOGDIR"] = args.logdir
    os.environ["AUTOMLBOARD_LOGLEVEL"] = args.log_level
    os.environ["AUTOMLBOARD_RELOAD_INTERVAL"] = str(args.reload_interval)

    if args.db:
        try:
            db_address_reg = re.compile(r"(.*)://(.*):(.*)@(.*):(.*)/(.*)")
            match = re.match(db_address_reg, args.db_address)
            os.environ["AUTOMLBOARD_DB_ENGINE"] = match.group(1)
            os.environ["AUTOMLBOARD_DB_USER"] = match.group(2)
            os.environ["AUTOMLBOARD_DB_PASSWORD"] = match.group(3)
            os.environ["AUTOMLBOARD_DB_HOST"] = match.group(4)
            os.environ["AUTOMLBOARD_DB_PORT"] = match.group(5)
            os.environ["AUTOMLBOARD_DB_NAME"] = match.group(6)
            print("Using %s as the database backend." % match.group(1))
        except BaseException as e:
            raise DatabaseError(e)
    else:
        print(
            "Using sqlite3 as the database backend, "
            "information will be stored in automlboard.db")

    os.environ.setdefault("DJANGO_SETTINGS_MODULE", "frontend.settings")
    django.setup()
    command = ['manage.py', 'migrate', '--run-syncdb']
    execute_from_command_line(command)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--logdir",
                        type=str,
                        required=True,
                        help="Directory where AutoML Board will "
                             "look to find tuning logs it can display")
    parser.add_argument("--port",
                        type=int,
                        default=8008,
                        help="What port to serve AutoMLBoard on, "
                             "(default: %(default)s)")
    parser.add_argument("--db",
                        type=str,
                        default=None,
                        help="Set SQL database URI in "
                             "schema://user:password@host:port/database, "
                             "(default: sqlite3)"),
    parser.add_argument("--reload_interval",
                        type=int,
                        default=5,
                        help="How often the backend should load more data, "
                             "(default: %(default)s)")
    parser.add_argument("--log_level",
                        type=str,
                        default="INFO",
                        help="Set the logging level, "
                             "(default: %(default)s)")
    parser.add_argument("--share_mode",
                        action="store_true",
                        help="parse logdir as the shared parent directory "
                             "of all jobs.")
    cmd_args = parser.parse_args()

    run_board(cmd_args)
