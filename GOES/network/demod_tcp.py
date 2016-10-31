#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: LRIT Demodulator TCP Pipe
# Author: Lucas Teske
# Generated: Sun Oct 30 17:16:15 2016
##################################################

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

from gnuradio import analog
from gnuradio import blocks
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio import wxgui
from gnuradio.eng_option import eng_option
from gnuradio.fft import window
from gnuradio.filter import firdes
from gnuradio.wxgui import fftsink2
from gnuradio.wxgui import forms
from gnuradio.wxgui import scopesink2
from gnuradio.wxgui import waterfallsink2
from grc_gnuradio import blks2 as grc_blks2
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
import osmosdr
import time
import wx


class demod_tcp(grc_wxgui.top_block_gui):

    def __init__(self):
        grc_wxgui.top_block_gui.__init__(self, title="LRIT Demodulator TCP Pipe")
        _icon_path = "/usr/share/icons/hicolor/32x32/apps/gnuradio-grc.png"
        self.SetIcon(wx.Icon(_icon_path, wx.BITMAP_TYPE_ANY))

        ##################################################
        # Variables
        ##################################################
        self.symbol_rate = symbol_rate = 293883
        self.samp_rate = samp_rate = 10e6/8
        self.vgagain = vgagain = 15
        self.sps = sps = (samp_rate*1.0)/(symbol_rate*1.0)
        self.pll_alpha = pll_alpha = 0.00199
        self.mixgain = mixgain = 15
        self.lnagain = lnagain = 15
        self.clock_alpha = clock_alpha = 0.0037
        self.center_freq = center_freq = 1691e6

        ##################################################
        # Blocks
        ##################################################
        _vgagain_sizer = wx.BoxSizer(wx.VERTICAL)
        self._vgagain_text_box = forms.text_box(
        	parent=self.GetWin(),
        	sizer=_vgagain_sizer,
        	value=self.vgagain,
        	callback=self.set_vgagain,
        	label='VGA Gain',
        	converter=forms.int_converter(),
        	proportion=0,
        )
        self._vgagain_slider = forms.slider(
        	parent=self.GetWin(),
        	sizer=_vgagain_sizer,
        	value=self.vgagain,
        	callback=self.set_vgagain,
        	minimum=0,
        	maximum=15,
        	num_steps=15,
        	style=wx.SL_HORIZONTAL,
        	cast=int,
        	proportion=1,
        )
        self.GridAdd(_vgagain_sizer, 4, 1, 1, 1)
        _pll_alpha_sizer = wx.BoxSizer(wx.VERTICAL)
        self._pll_alpha_text_box = forms.text_box(
        	parent=self.GetWin(),
        	sizer=_pll_alpha_sizer,
        	value=self.pll_alpha,
        	callback=self.set_pll_alpha,
        	label='PLL Alpha',
        	converter=forms.float_converter(),
        	proportion=0,
        )
        self._pll_alpha_slider = forms.slider(
        	parent=self.GetWin(),
        	sizer=_pll_alpha_sizer,
        	value=self.pll_alpha,
        	callback=self.set_pll_alpha,
        	minimum=0.001,
        	maximum=0.1,
        	num_steps=1000,
        	style=wx.SL_HORIZONTAL,
        	cast=float,
        	proportion=1,
        )
        self.GridAdd(_pll_alpha_sizer, 6, 1, 1, 1)
        _mixgain_sizer = wx.BoxSizer(wx.VERTICAL)
        self._mixgain_text_box = forms.text_box(
        	parent=self.GetWin(),
        	sizer=_mixgain_sizer,
        	value=self.mixgain,
        	callback=self.set_mixgain,
        	label='Mixer Gain',
        	converter=forms.int_converter(),
        	proportion=0,
        )
        self._mixgain_slider = forms.slider(
        	parent=self.GetWin(),
        	sizer=_mixgain_sizer,
        	value=self.mixgain,
        	callback=self.set_mixgain,
        	minimum=0,
        	maximum=15,
        	num_steps=15,
        	style=wx.SL_HORIZONTAL,
        	cast=int,
        	proportion=1,
        )
        self.GridAdd(_mixgain_sizer, 3, 1, 1, 1)
        _lnagain_sizer = wx.BoxSizer(wx.VERTICAL)
        self._lnagain_text_box = forms.text_box(
        	parent=self.GetWin(),
        	sizer=_lnagain_sizer,
        	value=self.lnagain,
        	callback=self.set_lnagain,
        	label='LNA Gain',
        	converter=forms.int_converter(),
        	proportion=0,
        )
        self._lnagain_slider = forms.slider(
        	parent=self.GetWin(),
        	sizer=_lnagain_sizer,
        	value=self.lnagain,
        	callback=self.set_lnagain,
        	minimum=0,
        	maximum=15,
        	num_steps=15,
        	style=wx.SL_HORIZONTAL,
        	cast=int,
        	proportion=1,
        )
        self.GridAdd(_lnagain_sizer, 2, 1, 1, 1)
        _clock_alpha_sizer = wx.BoxSizer(wx.VERTICAL)
        self._clock_alpha_text_box = forms.text_box(
        	parent=self.GetWin(),
        	sizer=_clock_alpha_sizer,
        	value=self.clock_alpha,
        	callback=self.set_clock_alpha,
        	label='Clock Alpha',
        	converter=forms.float_converter(),
        	proportion=0,
        )
        self._clock_alpha_slider = forms.slider(
        	parent=self.GetWin(),
        	sizer=_clock_alpha_sizer,
        	value=self.clock_alpha,
        	callback=self.set_clock_alpha,
        	minimum=0.001,
        	maximum=0.01,
        	num_steps=10,
        	style=wx.SL_HORIZONTAL,
        	cast=float,
        	proportion=1,
        )
        self.GridAdd(_clock_alpha_sizer, 5, 1, 1, 1)
        self.wxgui_waterfallsink2_0 = waterfallsink2.waterfall_sink_c(
        	self.GetWin(),
        	baseband_freq=center_freq,
        	dynamic_range=10,
        	ref_level=-40,
        	ref_scale=2,
        	sample_rate=samp_rate,
        	fft_size=16384,
        	fft_rate=15,
        	average=False,
        	avg_alpha=None,
        	title='Input Waterfall',
        	win=window.hamming,
        )
        self.GridAdd(self.wxgui_waterfallsink2_0.win, 0, 0, 1, 1)
        self.wxgui_scopesink2_0_0 = scopesink2.scope_sink_c(
        	self.GetWin(),
        	title='BPSK Constellation',
        	sample_rate=symbol_rate,
        	v_scale=0.5,
        	v_offset=0,
        	t_scale=0.5,
        	ac_couple=False,
        	xy_mode=True,
        	num_inputs=1,
        	trig_mode=wxgui.TRIG_MODE_AUTO,
        	y_axis_label='Counts',
        )
        self.GridAdd(self.wxgui_scopesink2_0_0.win, 2, 0, 6, 1)
        self.wxgui_fftsink2_0 = fftsink2.fft_sink_c(
        	self.GetWin(),
        	baseband_freq=1691e6,
        	y_per_div=1,
        	y_divs=10,
        	ref_level=-35,
        	ref_scale=4.0,
        	sample_rate=samp_rate,
        	fft_size=1024,
        	fft_rate=30,
        	average=True,
        	avg_alpha=0.0339,
        	title='Input FFT',
        	peak_hold=True,
        	win=window.hamming,
        )
        self.GridAdd(self.wxgui_fftsink2_0.win, 0, 1, 1, 1)
        self.root_raised_cosine_filter_0 = filter.fir_filter_ccf(1, firdes.root_raised_cosine(
        	1, samp_rate, symbol_rate, 0.5, 361))
        self.rational_resampler_xxx_0 = filter.rational_resampler_ccc(
                interpolation=15,
                decimation=18,
                taps=None,
                fractional_bw=None,
        )
        self.osmosdr_source_0_0 = osmosdr.source( args="numchan=" + str(1) + " " + 'airspy=0' )
        self.osmosdr_source_0_0.set_sample_rate(3e6)
        self.osmosdr_source_0_0.set_center_freq(center_freq, 0)
        self.osmosdr_source_0_0.set_freq_corr(0, 0)
        self.osmosdr_source_0_0.set_dc_offset_mode(1, 0)
        self.osmosdr_source_0_0.set_iq_balance_mode(1, 0)
        self.osmosdr_source_0_0.set_gain_mode(False, 0)
        self.osmosdr_source_0_0.set_gain(lnagain, 0)
        self.osmosdr_source_0_0.set_if_gain(vgagain, 0)
        self.osmosdr_source_0_0.set_bb_gain(mixgain, 0)
        self.osmosdr_source_0_0.set_antenna('', 0)
        self.osmosdr_source_0_0.set_bandwidth(0, 0)
          
        self.low_pass_filter_0_0 = filter.fir_filter_ccf(2, firdes.low_pass(
        	1, samp_rate*2, symbol_rate * 2, 50e3, firdes.WIN_KAISER, 6.76))
        self.digital_costas_loop_cc_0 = digital.costas_loop_cc(pll_alpha, 2, False)
        self.digital_clock_recovery_mm_xx_0 = digital.clock_recovery_mm_cc(sps, clock_alpha**2/4.0, 0.5, clock_alpha, 0.005)
        self.blocks_stream_to_vector_0 = blocks.stream_to_vector(gr.sizeof_char*1, 16)
        self.blocks_null_sink_0 = blocks.null_sink(gr.sizeof_char*16)
        self.blocks_float_to_char_0 = blocks.float_to_char(1, 127)
        self.blocks_complex_to_real_0 = blocks.complex_to_real(1)
        self.blks2_tcp_sink_0 = grc_blks2.tcp_sink(
        	itemsize=gr.sizeof_char*16,
        	addr='127.0.0.1',
        	port=5000,
        	server=False,
        )
        self.analog_agc_xx_0 = analog.agc_cc(100e-4, 0.5, 0.5)
        self.analog_agc_xx_0.set_max_gain(4000)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_agc_xx_0, 0), (self.root_raised_cosine_filter_0, 0))    
        self.connect((self.analog_agc_xx_0, 0), (self.wxgui_fftsink2_0, 0))    
        self.connect((self.analog_agc_xx_0, 0), (self.wxgui_waterfallsink2_0, 0))    
        self.connect((self.blocks_complex_to_real_0, 0), (self.blocks_float_to_char_0, 0))    
        self.connect((self.blocks_float_to_char_0, 0), (self.blocks_stream_to_vector_0, 0))    
        self.connect((self.blocks_stream_to_vector_0, 0), (self.blks2_tcp_sink_0, 0))    
        self.connect((self.blocks_stream_to_vector_0, 0), (self.blocks_null_sink_0, 0))    
        self.connect((self.digital_clock_recovery_mm_xx_0, 0), (self.blocks_complex_to_real_0, 0))    
        self.connect((self.digital_clock_recovery_mm_xx_0, 0), (self.wxgui_scopesink2_0_0, 0))    
        self.connect((self.digital_costas_loop_cc_0, 0), (self.digital_clock_recovery_mm_xx_0, 0))    
        self.connect((self.low_pass_filter_0_0, 0), (self.analog_agc_xx_0, 0))    
        self.connect((self.osmosdr_source_0_0, 0), (self.rational_resampler_xxx_0, 0))    
        self.connect((self.rational_resampler_xxx_0, 0), (self.low_pass_filter_0_0, 0))    
        self.connect((self.root_raised_cosine_filter_0, 0), (self.digital_costas_loop_cc_0, 0))    

    def get_symbol_rate(self):
        return self.symbol_rate

    def set_symbol_rate(self, symbol_rate):
        self.symbol_rate = symbol_rate
        self.set_sps((self.samp_rate*1.0)/(self.symbol_rate*1.0))
        self.wxgui_scopesink2_0_0.set_sample_rate(self.symbol_rate)
        self.root_raised_cosine_filter_0.set_taps(firdes.root_raised_cosine(1, self.samp_rate, self.symbol_rate, 0.5, 361))
        self.low_pass_filter_0_0.set_taps(firdes.low_pass(1, self.samp_rate*2, self.symbol_rate * 2, 50e3, firdes.WIN_KAISER, 6.76))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.set_sps((self.samp_rate*1.0)/(self.symbol_rate*1.0))
        self.wxgui_waterfallsink2_0.set_sample_rate(self.samp_rate)
        self.wxgui_fftsink2_0.set_sample_rate(self.samp_rate)
        self.root_raised_cosine_filter_0.set_taps(firdes.root_raised_cosine(1, self.samp_rate, self.symbol_rate, 0.5, 361))
        self.low_pass_filter_0_0.set_taps(firdes.low_pass(1, self.samp_rate*2, self.symbol_rate * 2, 50e3, firdes.WIN_KAISER, 6.76))

    def get_vgagain(self):
        return self.vgagain

    def set_vgagain(self, vgagain):
        self.vgagain = vgagain
        self._vgagain_slider.set_value(self.vgagain)
        self._vgagain_text_box.set_value(self.vgagain)
        self.osmosdr_source_0_0.set_if_gain(self.vgagain, 0)

    def get_sps(self):
        return self.sps

    def set_sps(self, sps):
        self.sps = sps
        self.digital_clock_recovery_mm_xx_0.set_omega(self.sps)

    def get_pll_alpha(self):
        return self.pll_alpha

    def set_pll_alpha(self, pll_alpha):
        self.pll_alpha = pll_alpha
        self._pll_alpha_slider.set_value(self.pll_alpha)
        self._pll_alpha_text_box.set_value(self.pll_alpha)
        self.digital_costas_loop_cc_0.set_loop_bandwidth(self.pll_alpha)

    def get_mixgain(self):
        return self.mixgain

    def set_mixgain(self, mixgain):
        self.mixgain = mixgain
        self._mixgain_slider.set_value(self.mixgain)
        self._mixgain_text_box.set_value(self.mixgain)
        self.osmosdr_source_0_0.set_bb_gain(self.mixgain, 0)

    def get_lnagain(self):
        return self.lnagain

    def set_lnagain(self, lnagain):
        self.lnagain = lnagain
        self._lnagain_slider.set_value(self.lnagain)
        self._lnagain_text_box.set_value(self.lnagain)
        self.osmosdr_source_0_0.set_gain(self.lnagain, 0)

    def get_clock_alpha(self):
        return self.clock_alpha

    def set_clock_alpha(self, clock_alpha):
        self.clock_alpha = clock_alpha
        self._clock_alpha_slider.set_value(self.clock_alpha)
        self._clock_alpha_text_box.set_value(self.clock_alpha)
        self.digital_clock_recovery_mm_xx_0.set_gain_omega(self.clock_alpha**2/4.0)
        self.digital_clock_recovery_mm_xx_0.set_gain_mu(self.clock_alpha)

    def get_center_freq(self):
        return self.center_freq

    def set_center_freq(self, center_freq):
        self.center_freq = center_freq
        self.wxgui_waterfallsink2_0.set_baseband_freq(self.center_freq)
        self.osmosdr_source_0_0.set_center_freq(self.center_freq, 0)


def main(top_block_cls=demod_tcp, options=None):
    if gr.enable_realtime_scheduling() != gr.RT_OK:
        print "Error: failed to enable real-time scheduling."

    tb = top_block_cls()
    tb.Start(True)
    tb.Wait()


if __name__ == '__main__':
    main()
