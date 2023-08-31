WREN_LIBS = [
	"gamelib.wren",
	"gamelib_test.wren"
]

import sys, os
from pathlib import Path

def process_to_c_string(file_name):
	processed = ""
	with open(file_name, "r") as fp:
		for line in fp:
			line = line.strip("\n")
			line = line.replace('"', '\\"')
			processed += f'"{line}\\n"\n'
	return processed

for fn in WREN_LIBS:
	fname = Path(fn).stem
	with open(f"wren_{fname}.h", "w") as fp:
		fp.write(process_to_c_string(fn))
