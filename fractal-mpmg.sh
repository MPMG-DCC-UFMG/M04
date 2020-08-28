#!/usr/bin/env bash

printf "Description: Script launcher for Fractal MPMG applications\n\n"

usage="
Usage:
config=<config-file-path>.json ... $(basename "$0")"

required="config"

if [ -z "$M06_HOME" ]; then
  echo "$M06_HOME is unset"
  exit 1
else
  echo "info: $M06_HOME is set to $M06_HOME"
fi

if [ -z "$SPARK_HOME" ]; then
  echo "SPARK_HOME is unset"
  exit 1
else
  echo "info: SPARK_HOME is set to $SPARK_HOME"
fi

wholeusage="$usage"

for argname in $required; do
	if [ -z ${!argname+x} ]; then
		printf "error: $argname is unset\n"
                printf "$wholeusage\n"
		exit 1
	else
		echo "info: $argname is set to '${!argname}'"
	fi
done

cmd="$SPARK_HOME/bin/spark-submit \\
   --class br.ufmg.cs.systems.fractal.mpmg.MPMGSparkRunner \\
   $M06_HOME/build/libs/M06.jar \\
   $config"

printf "info: Submitting command:\n$cmd\n\n"
bash -c "$cmd"
