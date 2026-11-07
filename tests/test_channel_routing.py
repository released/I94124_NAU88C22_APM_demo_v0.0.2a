"""Check real I2C output packing and the CPU refill of SPI TX PDMA rows.

Requires Python 3 and Visual Studio C++ build tools. APM and IRQ operations
are mocked; no algorithm quality, physical I2S slot order or timing is tested.
Generated evidence is retained under the ignored .local tree.
"""
import json
import shutil
import subprocess

from test_stream_state import ROOT, SAMPLE, compiler_environment, function


SUPPORT = r'''
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_handler.h"
#define FALSE 0
#define PDMA_IRQn 1
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static DATA_MANAGER g_sDataManager;
static unsigned int apm_calls, frame_number, irq_masked;
static uint16_t near_samples[COLSONIC_DAT_LEN], far_samples[COLSONIC_DAT_LEN];
static uint16_t sample(unsigned int i, unsigned int channel)
{
    static const uint16_t edge[] = {0, 1, 0x7FFF, 0x8000, 0x8001, 0xFFFF};
    return i < 6 ? edge[(i+channel)%6] : (uint16_t)(i*997U+channel*3011U+frame_number*53U);
}
static void Colsonic_apm_proc(int16_t *near, int16_t *far, int16_t *decode)
{
    unsigned int i;
    CHECK(near == (int16_t *)near_samples);
    CHECK(far == (int16_t *)far_samples);
    CHECK(decode == (int16_t *)g_sDataManager.u16DecodeTmp);
    for (i=0; i<COLSONIC_DAT_LEN; ++i)
    {
        CHECK(((uint16_t *)near)[i] == sample(i, 0));
        CHECK(((uint16_t *)far)[i] == sample(i, 1));
        ((uint16_t *)decode)[i] = sample(i, 2);
    }
    ++apm_calls;
}
static void NVIC_DisableIRQ(int irq) { CHECK(irq == PDMA_IRQn && !irq_masked); irq_masked=1; }
static void NVIC_EnableIRQ(int irq) { CHECK(irq == PDMA_IRQn && irq_masked); irq_masked=0; }
'''

CASES = r'''
static void check_channels(uint32_t word, unsigned int sample_index, int bypass)
{
    uint16_t left = (uint16_t)(word & 0xFFFFU);
    uint16_t right = (uint16_t)(word >> 16);
    CHECK(left == sample(sample_index, bypass ? 1 : 0));
    CHECK(right == sample(sample_index, bypass ? 0 : 2));
}
int main(void)
{
    unsigned int i, block, row, mode, queue_row, tx_row, read_index;
    g_sDataManager.pu16Near=near_samples;
    g_sDataManager.pu16Far=far_samples;
    Colsonic_Work_Handle();
    CHECK(apm_calls == 0 && g_sDataManager.u16DecodeAvalCnt == 0);
    CHECK(g_sDataManager.u8Decode_Buf_Idx == 0);
    puts("PASS no processing or queue publication before Near ready");
    for (frame_number=0; frame_number<4; ++frame_number)
    {
        mode=frame_number%2;
        queue_row=g_sDataManager.u8Decode_Buf_Idx;
        for (i=0; i<COLSONIC_DAT_LEN; ++i)
        {
            near_samples[i]=sample(i, 0);
            far_samples[i]=sample(i, 1);
        }
        g_sDataManager.bByPass=(uint8_t)mode;
        g_sDataManager.bNearReady=1;
        Colsonic_Work_Handle();
        CHECK(apm_calls == frame_number+1 && !irq_masked);
        CHECK(!g_sDataManager.bNearReady && !g_sDataManager.bFarReady);
        CHECK(g_sDataManager.u16DecodeAvalCnt == COLSONIC_DAT_LEN);
        CHECK(g_sDataManager.u8Decode_Buf_Idx == (queue_row^1U));
        for (i=0; i<COLSONIC_DAT_LEN; ++i)
        {
            check_channels(g_sDataManager.u32Decode[queue_row][i], i, mode);
            CHECK(near_samples[i] == sample(i, 0) && far_samples[i] == sample(i, 1));
        }
        for (block=0; block<COLSONIC_DAT_LEN/I2S_TX_BUF_LEN; ++block)
        {
            row=g_sDataManager.u16SPI_I2S_Tx_Buff_WriteIndex;
            pdma_spi_i2s_tx_handler(&g_sDataManager);
            CHECK(g_sDataManager.u16SPI_I2S_Tx_Buff_WriteIndex == (row^1U));
            CHECK(g_sDataManager.u16DecodeAvalCnt == COLSONIC_DAT_LEN-(block+1)*I2S_TX_BUF_LEN);
            for (i=0; i<I2S_TX_BUF_LEN; ++i)
                check_channels(g_sDataManager.au32SPI_I2S_Tx_Buff[row][i], block*I2S_TX_BUF_LEN+i, mode);
        }
        CHECK(g_sDataManager.u16DecodeBufRIdx == ((frame_number+1)%2)*COLSONIC_DAT_LEN);
        printf("PASS frame %u: %s, 320 samples through ten PDMA rows\n", frame_number, mode ? "bypass Far/Near" : "normal Near/Decode");
    }
    read_index=g_sDataManager.u16DecodeBufRIdx;
    tx_row=g_sDataManager.u16SPI_I2S_Tx_Buff_WriteIndex;
    memset(g_sDataManager.au32SPI_I2S_Tx_Buff[tx_row], 0xA5, sizeof(g_sDataManager.au32SPI_I2S_Tx_Buff[tx_row]));
    pdma_spi_i2s_tx_handler(&g_sDataManager);
    for (i=0; i<I2S_TX_BUF_LEN; ++i) CHECK(g_sDataManager.au32SPI_I2S_Tx_Buff[tx_row][i] == 0);
    CHECK(g_sDataManager.u16DecodeBufRIdx == read_index && g_sDataManager.u16DecodeAvalCnt == 0);
    CHECK(g_sDataManager.u16SPI_I2S_Tx_Buff_WriteIndex == (tx_row^1U));
    puts("PASS underrun zero fill, queue wrap and normal/bypass switching");
    return 0;
}
'''


def main():
    output = ROOT / '.local/tests/channel_routing'
    output.mkdir(parents=True, exist_ok=True)
    misc = (SAMPLE / 'Util/misc.c').read_text(encoding='utf-8-sig')
    pdma = (SAMPLE / 'Util/pdma_handler.c').read_text(encoding='utf-8-sig')
    body = function(misc, 'Colsonic_Work_Handle')
    body += '\n' + function(pdma, 'pdma_spi_i2s_tx_handler')
    normal = ('((uint32_t)g_sDataManager.u16DecodeTmp[i] << 16) |\n'
              '                    (uint32_t)g_sDataManager.pu16Near[i]')
    bypass = ('((uint32_t)g_sDataManager.pu16Near[i] << 16) |\n'
              '                    (uint32_t)g_sDataManager.pu16Far[i]')
    variants = {'current': body,
        'reversed_normal': body.replace(normal, '((uint32_t)g_sDataManager.pu16Near[i] << 16) | g_sDataManager.u16DecodeTmp[i]'),
        'reversed_bypass': body.replace(bypass, '((uint32_t)g_sDataManager.pu16Far[i] << 16) | g_sDataManager.pu16Near[i]')}
    assert all(value != body for name, value in variants.items() if name != 'current')
    environment = compiler_environment()
    environment['VSLANG'] = '1033'
    compiler = shutil.which('cl.exe', path=environment['PATH'])
    if not compiler:
        raise RuntimeError('Visual Studio C compiler not found')
    results = []
    for label, selected in variants.items():
        source = output / (label+'.c')
        source.write_text(SUPPORT+'\n'+selected+'\n'+CASES, encoding='ascii')
        executable = output / (label+'.exe')
        built = subprocess.run([compiler, '/nologo', '/TC', '/W3', '/I'+str(SAMPLE/'Util'),
            '/I'+str(SAMPLE), '/Fe:'+str(executable), '/Fo:'+str(output/(label+'.obj')),
            str(source)], env=environment, capture_output=True, text=True, errors='replace')
        (output/(label+'_build.log')).write_text(built.stdout+built.stderr, encoding='utf-8')
        if built.returncode:
            raise RuntimeError(label+' compilation failed:\n'+built.stdout+built.stderr)
        tested = subprocess.run([str(executable)], capture_output=True, text=True, errors='replace')
        (output/(label+'_test.log')).write_text(tested.stdout+tested.stderr, encoding='utf-8')
        negative = label != 'current'
        passed = tested.returncode != 0 if negative else tested.returncode == 0
        if not passed or (negative and 'left == sample' not in tested.stderr):
            raise AssertionError(label+' unexpected result:\n'+tested.stdout+tested.stderr)
        results.append({'case': label, 'result': 'PASS', 'negative_control': negative,
                        'program_exit_code': tested.returncode})
        print('PASS', label, '(channel swap rejected)' if negative else '(packing, PDMA refill, wrap, mode switch and zero fill)')
    (output/'results.json').write_text(json.dumps(results, indent=2)+'\n', encoding='utf-8')


if __name__ == '__main__':
    main()
