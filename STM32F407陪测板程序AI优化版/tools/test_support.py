# -*- coding: utf-8 -*-
"""Host regression tests for real data_processing.c and w25q32.c (Python 2/3)."""
from __future__ import print_function
import io
import os
import re
import subprocess
import sys

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(root)
out = os.path.join('0_readme', 'validation', 'support_host')
if not os.path.isdir(out):
    os.makedirs(out)

for source in ('data_processing', 'w25q32'):
    text = io.open('6_hardware/' + source + '.c', encoding='gbk').read()
    # Replace only hardware/project includes. The complete production function bodies run.
    text = re.sub(r'^#include "[^"]+"\s*$', '', text, flags=re.M)
    with io.open(os.path.join(out, source + '_under_test.c'), 'w', encoding='utf-8') as handle:
        handle.write(text)

compiler = sys.argv[1] if len(sys.argv) > 1 else 'gcc'
exe = os.path.join(out, 'test_support.exe' if os.name == 'nt' else 'test_support')
command = [compiler, '-std=c99', '-O2', '-Wall', '-Wextra', '-Werror',
           '-I' + out, 'tools/test_support.c', '-o', exe]
subprocess.check_call(command)
result = subprocess.check_output([os.path.abspath(exe)])
if not isinstance(result, str):
    result = result.decode('utf-8')
print(result.strip())
with io.open(os.path.join(out, 'results.txt'), 'w', encoding='utf-8') as handle:
    handle.write(result.decode('utf-8') if isinstance(result, bytes) else result)
