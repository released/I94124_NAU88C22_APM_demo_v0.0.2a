"""Exercise the real legacy lifecycle/command function bodies with mock I/O.

Requires Python 3 and Visual Studio C++ build tools on Windows. No board or
APM algorithm is executed. Generated files default to the ignored .local tree.
"""
from pathlib import Path
import argparse
import json
import os
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SAMPLE = ROOT / 'SampleCode/Template/Colsonic_16k_I2S_I2C'


def function(source, name):
    match = re.search(r'void\s+' + name + r'\s*\([^;{}]*\)\s*\{', source)
    if not match:
        raise AssertionError('Missing function: ' + name)
    depth = 0
    lex = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]', re.S)
    for token in lex.finditer(source, match.end()-1):
        if token.group() == '{':
            depth += 1
        elif token.group() == '}':
            depth -= 1
            if depth == 0:
                return source[match.start():token.end()]
    raise AssertionError('Unclosed function: ' + name)


def compiler_environment():
    vswhere = Path(os.environ['ProgramFiles(x86)']) / 'Microsoft Visual Studio/Installer/vswhere.exe'
    script = subprocess.check_output([str(vswhere), '-latest', '-products', '*',
        '-find', 'VC/Auxiliary/Build/vcvars64.bat'], text=True).strip()
    if not script:
        raise RuntimeError('Visual Studio C++ build tools not found')
    result = subprocess.run('call "' + script + '" >nul && set', shell=True,
                            capture_output=True, text=True, check=True)
    env = os.environ.copy()
    for line in result.stdout.splitlines():
        key, separator, value = line.partition('=')
        if separator and key:
            env[key.upper()] = value
    return env


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, default=ROOT / '.local/tests/stream_state')
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    misc = (SAMPLE / 'Util/misc.c').read_text(encoding='utf-8-sig')
    command = (SAMPLE / 'Util/i2c_handler.c').read_text(encoding='utf-8-sig')
    prefix = command[command.index('typedef struct _i2c_rec'):command.index('void reg_read_reply')]
    bodies = '\n\n'.join(function(misc, name) for name in ['Start_Audio', 'Stop_Audio', 'Misc'])
    bodies += '\n\n' + function(command, 'reg_read_reply')
    bodies += '\n\n' + function(command, 'I2C_Cmd_Handler')
    support = r'''
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "data_handler.h"
#include "i2c_cmd.h"
#ifdef TEST_SPI1
#undef USB_EVB_SPI2_I2S
#endif
#define TRUE 1
#define FALSE 0
#define DEBUG
#define DSP_VER_MAJOR 0
#define DSP_VER_MINOR 1
#define USBD_IRQn 0
#define PDMA_IRQn 1
#define I2S0 0
#define SPI1 1
#define SPI2 2
#define I2C0 0
#define I2C1 1
#define I2S0_TX_PDMA_CH 1
#define I2S0_RX_PDMA_CH 0
#define SPI1_I2S_TX_PDMA_CH 5
#define SPI1_I2S_RX_PDMA_CH 4
#define SPI2_I2S_TX_PDMA_CH 7
#define SPI2_I2S_RX_PDMA_CH 6
typedef int BOOL;
typedef int I2C_T;
typedef struct { int tx, rx, txdma, rxdma, control; } TEST_PORT;
static TEST_PORT ports[3];
static unsigned int setups, apm_resets;
static uint8_t reply_byte;
static volatile uint8_t tmr3_flag;
DATA_MANAGER g_sDataManager;
static int test_printf(const char *format, ...) { (void)format; return 0; }
#define printf test_printf
#define I2C_SET_DATA(p,v) ((void)(p), reply_byte=(uint8_t)(v))
#define I2S_CLR_TX_FIFO(p) ((void)(p))
#define I2S_CLR_RX_FIFO(p) ((void)(p))
#define I2S_ENABLE_TXDMA(p) (ports[p].txdma=1)
#define I2S_ENABLE_RXDMA(p) (ports[p].rxdma=1)
#define I2S_ENABLE_TX(p) (ports[p].tx=1)
#define I2S_ENABLE_RX(p) (ports[p].rx=1)
#define I2S_DISABLE_TXDMA(p) (ports[p].txdma=0)
#define I2S_DISABLE_RXDMA(p) (ports[p].rxdma=0)
#define I2S_DISABLE_TX(p) (ports[p].tx=0)
#define I2S_DISABLE_RX(p) (ports[p].rx=0)
#define I2S_DISABLE(p) ((void)(p))
#define SPI_I2S_ENABLE_TXDMA I2S_ENABLE_TXDMA
#define SPI_I2S_ENABLE_RXDMA I2S_ENABLE_RXDMA
#define SPI_I2S_ENABLE_TX I2S_ENABLE_TX
#define SPI_I2S_ENABLE_RX I2S_ENABLE_RX
#define SPI_I2S_DISABLE_TXDMA I2S_DISABLE_TXDMA
#define SPI_I2S_DISABLE_RXDMA I2S_DISABLE_RXDMA
#define SPI_I2S_DISABLE_TX I2S_DISABLE_TX
#define SPI_I2S_DISABLE_RX I2S_DISABLE_RX
#define SPI_I2SEnableControl(p) (ports[p].control=1)
#define SPI_I2SDisableControl(p) (ports[p].control=0)
static void CODEC_I2S_Init_8822(uint32_t r,uint32_t b,uint32_t m)
{ (void)r; (void)b; (void)m; setups++; }
static void PDMA_I2S0_TX_Init(uint8_t c) { (void)c; }
static void PDMA_I2S0_RX_Init(uint8_t c) { (void)c; }
static void PDMA_SPI1_I2S_TX_Init(uint8_t c) { (void)c; }
static void PDMA_SPI1_I2S_RX_Init(uint8_t c) { (void)c; }
static void PDMA_SPI2_I2S_TX_Init(uint8_t c) { (void)c; }
static void PDMA_SPI2_I2S_RX_Init(uint8_t c) { (void)c; }
static void SPI1_I2S_Init(uint32_t r) { (void)r; }
static void SPI2_I2S_Init(uint32_t r) { (void)r; }
static void PDMA_Close(void) { }
static uint32_t Colsonic_apm_init(void) { apm_resets++; return 0; }
static void NVIC_SetPriority(int i,int p) { (void)i; (void)p; }
static void UART0_Init(void) { }
static void I2C0_Slave_Init(void) { }
static void NAU88C22_Read(I2C_T *p,uint8_t *d,uint16_t r)
{ (void)p; (void)r; d[0]=0; d[1]=0; }
static void Set_NAU88C22_Reg(I2C_T *p,uint8_t r,uint16_t v)
{ (void)p; (void)r; (void)v; }
static void Set_AEC_ByPass(uint8_t v) { g_sDataManager.bByPass=v; }
void Start_Audio(void);
void Stop_Audio(void);
'''
    cases = r'''
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#c); return 1; } } while (0)
static void sys_command(uint8_t value)
{
    static uint8_t data;
    data=value;
    s_i2c_cmd.u8Cmd=SYS_CMD;
    s_i2c_cmd.pu8Dat=&data;
    s_i2c_cmd.bAval=TRUE;
    I2C_Cmd_Handler();
}
static int port_running(int p)
{ return ports[p].tx && ports[p].rx && ports[p].txdma && ports[p].rxdma; }
static int port_stopped(int p)
{ return !ports[p].tx && !ports[p].rx && !ports[p].txdma && !ports[p].rxdma; }
int main(void)
{
    unsigned int before;
#ifdef USB_EVB_SPI2_I2S
    int host_port=SPI2;
#else
    int host_port=SPI1;
#endif
    CHECK(g_bStreaming==AUDIO_PATH_STOP);
    Misc();
    reg_read_reply(I2C0,SYS_CMD);
    CHECK(g_bStreaming==AUDIO_PATH_START && reply_byte==1);
    CHECK(port_running(I2S0) && port_running(host_port));
    puts("PASS boot auto-start and one-byte status readback");
    before=apm_resets;
    sys_command(AUDIO_PATH_STOP);
    reg_read_reply(I2C0,SYS_CMD);
    CHECK(g_bStreaming==AUDIO_PATH_STOP && reply_byte==0);
    CHECK(port_stopped(I2S0) && port_stopped(host_port));
    CHECK(!ports[host_port].control && apm_resets==before+1);
    puts("PASS first STOP after boot tears down transport and resets APM");
    before=apm_resets;
    sys_command(AUDIO_PATH_STOP);
    CHECK(apm_resets==before);
    puts("PASS repeated STOP is idempotent");
    sys_command(AUDIO_PATH_START);
    reg_read_reply(I2C0,SYS_CMD);
    CHECK(reply_byte==1 && port_running(I2S0) && port_running(host_port));
    before=setups;
    sys_command(AUDIO_PATH_START);
    CHECK(setups==before);
    puts("PASS START readback and repeated START idempotence");
    sys_command(2);
    CHECK(g_bStreaming==AUDIO_PATH_START && setups==before);
    puts("PASS unsupported SYS_CMD value leaves state unchanged");
    Stop_Audio();
    reg_read_reply(I2C0,SYS_CMD);
    CHECK(reply_byte==0 && port_stopped(I2S0) && port_stopped(host_port));
    Start_Audio();
    reg_read_reply(I2C0,SYS_CMD);
    CHECK(reply_byte==1 && port_running(I2S0) && port_running(host_port));
    puts("PASS direct lifecycle calls also publish state");
    return 0;
}
'''
    environment = compiler_environment()
    environment['VSLANG'] = '1033'
    results = []
    for spi1 in (False, True):
        for negative in (False, True):
            label = ('spi1' if spi1 else 'spi2') + ('_negative' if negative else '')
            selected = bodies
            if negative:
                # Reintroduce the original ownership bug as a negative control.
                selected = selected.replace('g_bStreaming = AUDIO_PATH_START;', '')
                selected = selected.replace('g_bStreaming = AUDIO_PATH_STOP;', '')
                current_handler = function(command, 'I2C_Cmd_Handler')
                old_handler = current_handler.replace('Start_Audio();', 'Start_Audio(); g_bStreaming = AUDIO_PATH_START;')
                old_handler = old_handler.replace('Stop_Audio();', 'Stop_Audio(); g_bStreaming = AUDIO_PATH_STOP;')
                selected = selected.replace(current_handler, old_handler)
            source = output / (label + '.c')
            source.write_text(support + '\n' + prefix + '\n' + selected + '\n' + cases, encoding='ascii')
            executable = output / (label + '.exe')
            compiler = shutil.which('cl.exe', path=environment['PATH'])
            if not compiler:
                raise RuntimeError('C compiler not found in the Visual Studio environment')
            command_line = [compiler, '/nologo', '/TC', '/W3', '/wd4244', '/wd4101',
                            '/D_CRT_SECURE_NO_WARNINGS', '/I'+str(SAMPLE/'Util'),
                            '/I'+str(SAMPLE), '/Fe:'+str(executable),
                            '/Fo:'+str(output/(label+'.obj')), str(source)]
            if spi1:
                command_line.insert(6, '/DTEST_SPI1')
            built = subprocess.run(command_line, env=environment, capture_output=True,
                                   text=True, encoding='utf-8', errors='replace')
            (output/(label+'_build.log')).write_text(built.stdout+built.stderr, encoding='utf-8')
            if built.returncode:
                raise RuntimeError(label+' compilation failed:\n'+built.stdout+built.stderr)
            tested = subprocess.run([str(executable)], capture_output=True, text=True,
                                    encoding='utf-8', errors='replace')
            (output/(label+'_test.log')).write_text(tested.stdout+tested.stderr, encoding='utf-8')
            passed = tested.returncode != 0 if negative else tested.returncode == 0
            if not passed:
                raise AssertionError(label+' unexpected result:\n'+tested.stdout+tested.stderr)
            if negative and 'g_bStreaming==AUDIO_PATH_START && reply_byte==1' not in tested.stderr:
                raise AssertionError(label+' did not reproduce the boot status regression')
            results.append({'configuration':label,'result':'PASS',
                            'negative_control':negative,'program_exit_code':tested.returncode})
            print('PASS',label, '(original bug rejected)' if negative else '(six lifecycle checks)')
    (output/'results.json').write_text(json.dumps(results,indent=2)+'\n', encoding='utf-8')


if __name__ == '__main__':
    main()
