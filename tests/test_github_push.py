"""Offline checks for github_push.bat; creates only ignored local fixtures."""
from pathlib import Path
import argparse
import json
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, default=ROOT / '.local/tests/github_push')
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    fixture = Path(tempfile.mkdtemp(prefix='run_', dir=args.output))
    (fixture / 'tools').mkdir()
    for name in ['github_push.bat', '.gitignore', 'tools/github_push.ps1', 'tools/check_apm_consistency.ps1']:
        shutil.copyfile(ROOT / name, fixture / name)
    for sample, stem in [('Colsonic_16k_I2S_I2C', 'Colsonic_16k'), ('Nuvoton_demo_AEC_16k_0.0.2a', 'Colsonic_apm')]:
        source = ROOT / 'SampleCode/Template' / sample
        target = fixture / 'SampleCode/Template' / sample
        for directory, names in [
            ('Colsonic', ['colsonic_apm_16k_320.lib', 'colsonicdata.bin', 'colsonic_apm.h', 'colsonic_def.h', 'colsonic_def.c', 'alg_type.h', 'ColsonicData_Keil.s']),
            ('KEIL', [stem + '.uvprojx', 'startup_I94100.s'])
        ]:
            (target / directory).mkdir(parents=True)
            for name in names:
                shutil.copyfile(source / directory / name, target / directory / name)
    repository = 'example-owner/example-audio-project'
    (fixture / 'github_repository.json').write_text(json.dumps({
        'repository': repository, 'branch': 'main', 'visibility': 'private'
    }), encoding='utf-8')

    def git(*arguments):
        return subprocess.run(['git', '-C', str(fixture), *arguments], capture_output=True,
                              text=True, encoding='utf-8', errors='replace', check=True)

    git('init', '-b', 'main')
    git('config', 'user.name', 'Offline upload test')
    git('config', 'user.email', 'test@example.invalid')
    git('config', 'commit.gpgsign', 'false')
    git('config', 'core.autocrlf', 'false')
    results = []

    def check(label, success, expected_text):
        before = git('status', '--porcelain=v1', '--untracked-files=all').stdout
        result = subprocess.run('call "' + str(fixture / 'github_push.bat') + '" -CheckOnly',
                                cwd=fixture, shell=True, capture_output=True, text=True,
                                encoding='utf-8', errors='replace')
        text = result.stdout + result.stderr
        passed = (result.returncode == 0) == success and expected_text in text
        assert before == git('status', '--porcelain=v1', '--untracked-files=all').stdout
        assert passed, (label, result.returncode, text)
        (fixture / (label + '.log')).write_text(text, encoding='utf-8')
        # Test logs stay inside an ignored fixture directory in the real project.
        (fixture / '.gitignore').write_text((ROOT / '.gitignore').read_text(encoding='utf-8') + '\n/*.log\n', encoding='utf-8')
        results.append({'case': label, 'result': 'PASS', 'script_exit_code': result.returncode})
        print('PASS', label)

    check('unborn_without_origin', True, 'CheckOnly:')
    git('remote', 'add', 'origin', 'https://github.com/' + repository + '.git')
    check('matching_origin', True, 'CheckOnly:')
    git('remote', 'set-url', 'origin', 'https://github.com/another-owner/another-project.git')
    check('wrong_origin', False, 'Origin differs')
    git('remote', 'set-url', 'origin', 'https://github.com/' + repository + '.git')
    git('symbolic-ref', 'HEAD', 'refs/heads/work')
    check('wrong_branch', False, 'configured main branch')
    git('symbolic-ref', 'HEAD', 'refs/heads/main')
    private = fixture / 'docs/verification/test-only.txt'
    private.parent.mkdir(parents=True)
    private.write_text('Synthetic local engineering record', encoding='utf-8')
    git('add', '--force', '--', 'docs/verification/test-only.txt')
    check('tracked_private_record', False, 'Local/private content')
    git('rm', '--cached', '--', 'docs/verification/test-only.txt')
    git('add', '--', '.gitignore', 'github_repository.json', 'github_push.bat', 'tools/github_push.ps1')
    git('commit', '-m', 'Create offline fixture')
    git('add', '--force', '--', 'docs/verification/test-only.txt')
    git('commit', '-m', 'Add synthetic private record')
    git('rm', '--cached', '--', 'docs/verification/test-only.txt')
    git('commit', '-m', 'Remove record from current tree')
    check('private_history', False, 'Local/private content')
    (fixture / 'results.json').write_text(json.dumps(results, indent=2), encoding='utf-8')
    print('Offline results:', fixture / 'results.json')


if __name__ == '__main__':
    main()
