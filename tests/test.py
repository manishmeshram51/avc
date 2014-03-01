import yaml
import sys
import subprocess
import pdb
from substructure import substructure
def run_test(name, do_pmd2yaml):
    pmd_name = name + '.pmd'
    yaml_out = do_pmd2yaml(pmd_name)
    doc = yaml.load(yaml_out)
    expected_doc = yaml.load(open(name, 'r').read())
    return substructure(expected_doc, doc)

def get_yaml_text_reader(pmd2yaml):
    def get_yaml_text(name):
        pmd2yaml_process = subprocess.Popen([pmd2yaml, name], stdout=subprocess.PIPE)
        return subprocess.check_output(['sed', '-e', '1,/YAML follows/d'], stdin=pmd2yaml_process.stdout)
    return get_yaml_text

def _output_test_result(name, result):
    GREEN = '\033[92m'
    BLUE = '\033[94m'
    RED = '\033[91m'
    PURPLISH = '\033[95m'
    BOLD = '\033[1m'
    END = '\033[0m'

    result_color = GREEN if result == 'PASSED' else RED

    print PURPLISH + name + ':\n\t' + BOLD + result_color + result + END

def _get_test_names():
    return open('test_manifest', 'r').read().split('\n')

def _usage():
    print "python test.py /path/to/pmd2yaml"
    exit(5)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        _usage()
    yaml_text_reader = get_yaml_text_reader(sys.argv[1])
    failed = 0
    passed = 0
    errored = 0
    for test_name in _get_test_names():
        if not test_name:
            continue
        try:
            result = 'PASSED' if run_test(test_name, yaml_text_reader) else 'FAILED'
            if result == 'FAILED':
                failed = failed + 1
            else:
                passed = passed + 1
        except:
            result = 'ERROR'
            errored = errored + 1
        _output_test_result(test_name, result)
    print "Passed: %d, Failed: %d, Errored: %d" % (passed, failed, errored)
    if failed > 0 or errored > 0:
        exit(1)
