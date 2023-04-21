#!/bin/bash

# Define some log levels needed to specify how verbose this script is.
# For example, to change the log level to 'debug', type: "export LOG_LEVEL=debug; bash package.sh".
ERROR=4
WARN=3
INFO=2
DEBUG=1
TRACE=0
LOG_LEVEL=${LOG_LEVEL:-"trace"}
case "$LOG_LEVEL" in
  "error")
    LOG_LEVEL=$ERROR ;;
  "warn")
    LOG_LEVEL=$WARN ;;
  "info")
    LOG_LEVEL=$INFO ;;
  "debug")
    LOG_LEVEL=$DEBUG ;;
  "trace")
    LOG_LEVEL=$TRACE ;;
  *)
    echo "WARN: Invalid log level '$LOG_LEVEL'. Using log level 'info' instead."
    LOG_LEVEL=$INFO ;;
esac

# Define some ANSI codes needed to format the log messages.
RB="\033[31;1m"  # red+bold
GB="\033[32;1m"  # green+bold
BB="\033[34;1m"  # blue+bold
MB="\033[35;1m"  # magenta+bold
R="\033[31m"     # red
G="\033[32m"     # green
B="\033[34m"     # blue
MG="\033[35m"    # magenta
N="\033[0m"      # none

# Define the device to which the output of commands like "docker", "gpg" or "deb-scanpackages"
# should be sent. If the log level is "TRACE" or lower, define the device as "/dev/stdout" (so that
# the output of those commands is printed to the console). Otherwise, set the device to "/dev/null"
# (so that the output is not printed to the console).
[[ $LOG_LEVEL -le $TRACE ]] && DEV="/dev/stdout" || DEV="/dev/null"

# ==================================================================================================

# This function prints the specified debug message to the console, if the current log level is
# equal to "debug" or lower.
#
# Args:
#   $1 - The debug message to print.
#
# Example usage:
#   debug "Creating package ..."
#
function debug() {
  if [[ $LOG_LEVEL -le $DEBUG ]]; then
    echo "$1"
  fi
}

# This function prints the specified info message to the console (in bold), if the current log
# level is equal to "info" or lower.
#
# Args:
#   $1 - The info message to print.
#
# Example usage:
#   info_emph "Creating package ..."
#
function info_emph() {
  if [[ $LOG_LEVEL -le $INFO ]]; then
    echo -e "$MB$1$N"
  fi
}

# This function prints the specified info message to the console, if the current log level is
# equal to "info" or lower.
#
# Args:
#   $1 - The info message to print.
#
# Example usage:
#   info "Creating package ..."
#
function info() {
  if [[ $LOG_LEVEL -le $INFO ]]; then
    echo -e "$MG$1$N"
  fi
}

# This function prints the specified success message to the console (in bold), if the current log
# level is equal to "info" or lower.
#
# Args:
#   $1 - The success message to print.
#
# Example usage:
#   success_emph "Package created successfully."
#
function success_emph() {
  if [[ $LOG_LEVEL -le $INFO ]]; then
    echo -e "$GB$1$N"
  fi
}

# This function prints the specified success message to the console, if the current log level is
# equal to "info" or lower.
#
# Args:
#   $1 - The success message to print.
#
# Example usage:
#   success "Package created successfully."
#
function success() {
  if [[ $LOG_LEVEL -le $INFO ]]; then
    echo -e "$G$1$N"
  fi
}

# This function (a) prints the specified error message to the console, together with the function
# name and the line number specifying where the error occurred, if the current log level is
# equal to "error" or lower, and (b) terminates the execution of the script with status code 1.
#
# Args:
#   $1 - The error message to print.
#
# Example usage:
#   error "Parameter is missing."
#
function error() {
  if [[ $LOG_LEVEL -le $ERROR ]]; then
    echo -e "$RB$1 (in ${FUNCNAME[1]}:${BASH_LINENO[0]})$N"
  fi
  exit 1
}