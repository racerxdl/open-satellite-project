#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: LRIT Demodulator TCP Pipe
# Author: Lucas Teske
# Generated: Thu Oct 27 23:32:36 2016
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
from grc_gnuradio import blks2 as grc_blks2
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
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
        self.nb_0 = self.nb_0 = wx.Notebook(self.GetWin(), style=wx.NB_TOP)
        self.nb_0.AddPage(grc_wxgui.Panel(self.nb_0), "FFT")
        self.nb_0.AddPage(grc_wxgui.Panel(self.nb_0), "BPSK Constellation")
        self.Add(self.nb_0)
        _pll_alpha_sizer = wx.BoxSizer(wx.VERTICAL)
        self._pll_alpha_text_box = forms.text_box(
        	parent=self.nb_0.GetPage(1).GetWin(),
        	sizer=_pll_alpha_sizer,
        	value=self.pll_alpha,
        	callback=self.set_pll_alpha,
        	label='PLL Alpha',
        	converter=forms.float_converter(),
        	proportion=0,
        )
        self._pll_alpha_slider = forms.slider(
        	parent=self.nb_0.GetPage(1).GetWin(),
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
        self.nb_0.GetPage(1).Add(_pll_alpha_sizer)
        _clock_alpha_sizer = wx.BoxSizer(wx.VERTICAL)
        self._clock_alpha_text_box = forms.text_box(
        	parent=self.nb_0.GetPage(1).GetWin(),
        	sizer=_clock_alpha_sizer,
        	value=self.clock_alpha,
        	callback=self.set_clock_alpha,
        	label='Clock Alpha',
        	converter=forms.float_converter(),
        	proportion=0,
        )
        self._clock_alpha_slider = forms.slider(
        	parent=self.nb_0.GetPage(1).GetWin(),
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
        self.nb_0.GetPage(1).Add(_clock_alpha_sizer)
        self.wxgui_scopesink2_0_0 = scopesink2.scope_sink_c(
        	self.nb_0.GetPage(0).GetWin(),
        	title='Scope Plot',
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
        self.nb_0.GetPage(0).Add(self.wxgui_scopesink2_0_0.win)
        self.wxgui_scopesink2_0 = scopesink2.scope_sink_c(
        	self.nb_0.GetPage(1).GetWin(),
        	title='Scope Plot',
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
        self.nb_0.GetPage(1).Add(self.wxgui_scopesink2_0.win)
        self.wxgui_fftsink2_0 = fftsink2.fft_sink_c(
        	self.nb_0.GetPage(0).GetWin(),
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
        self.nb_0.GetPage(0).Add(self.wxgui_fftsink2_0.win)
        _vgagain_sizer = wx.BoxSizer(wx.VERTICAL)
        self._vgagain_text_box = forms.text_box(
        	parent=self.nb_0.GetPage(0).GetWin(),
        	sizer=_vgagain_sizer,
        	value=self.vgagain,
        	callback=self.set_vgagain,
        	label='VGA Gain',
        	converter=forms.int_converter(),
        	proportion=0,
        )
        self._vgagain_slider = forms.slider(
        	parent=self.nb_0.GetPage(0).GetWin(),
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
        self.nb_0.GetPage(0).Add(_vgagain_sizer)
        self.root_raised_cosine_filter_0 = filter.fir_filter_ccf(1, firdes.root_raised_cosine(
        	1, samp_rate, symbol_rate, 0.5, 361))
        _mixgain_sizer = wx.BoxSizer(wx.VERTICAL)
        self._mixgain_text_box = forms.text_box(
        	parent=self.nb_0.GetPage(0).GetWin(),
        	sizer=_mixgain_sizer,
        	value=self.mixgain,
        	callback=self.set_mixgain,
        	label='Mixer Gain',
        	converter=forms.int_converter(),
        	proportion=0,
        )
        self._mixgain_slider = forms.slider(
        	parent=self.nb_0.GetPage(0).GetWin(),
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
        self.nb_0.GetPage(0).Add(_mixgain_sizer)
        _lnagain_sizer = wx.BoxSizer(wx.VERTICAL)
        self._lnagain_text_box = forms.text_box(
        	parent=self.nb_0.GetPage(0).GetWin(),
        	sizer=_lnagain_sizer,
        	value=self.lnagain,
        	callback=self.set_lnagain,
        	label='LNA Gain',
        	converter=forms.int_converter(),
        	proportion=0,
        )
        self._lnagain_slider = forms.slider(
        	parent=self.nb_0.GetPage(0).GetWin(),
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
        self.nb_0.GetPage(0).Add(_lnagain_sizer)
        self.digital_costas_loop_cc_0 = digital.costas_loop_cc(pll_alpha, 2, False)
        self.digital_clock_recovery_mm_xx_0 = digital.clock_recovery_mm_cc(sps, clock_alpha**2/4.0, 0.5, clock_alpha, 0.005)
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_gr_complex*1, samp_rate,True)
        self.blocks_stream_to_vector_0 = blocks.stream_to_vector(gr.sizeof_char*1, 16)
        self.blocks_float_to_char_0 = blocks.float_to_char(1, 127)
        self.blocks_file_source_0 = blocks.file_source(gr.sizeof_gr_complex*1, '/media/ELTN/HAM/GOES/BaseBand/gqrx_20161024_184415_1691000000_1250000_fc.raw', False)
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
        self.connect((self.blocks_complex_to_real_0, 0), (self.blocks_float_to_char_0, 0))    
        self.connect((self.blocks_file_source_0, 0), (self.blocks_throttle_0, 0))    
        self.connect((self.blocks_float_to_char_0, 0), (self.blocks_stream_to_vector_0, 0))    
        self.connect((self.blocks_stream_to_vector_0, 0), (self.blks2_tcp_sink_0, 0))    
        self.connect((self.blocks_throttle_0, 0), (self.analog_agc_xx_0, 0))    
        self.connect((self.digital_clock_recovery_mm_xx_0, 0), (self.blocks_complex_to_real_0, 0))    
        self.connect((self.digital_clock_recovery_mm_xx_0, 0), (self.wxgui_scopesink2_0, 0))    
        self.connect((self.digital_clock_recovery_mm_xx_0, 0), (self.wxgui_scopesink2_0_0, 0))    
        self.connect((self.digital_costas_loop_cc_0, 0), (self.digital_clock_recovery_mm_xx_0, 0))    
        self.connect((self.root_raised_cosine_filter_0, 0), (self.digital_costas_loop_cc_0, 0))    

    def get_symbol_rate(self):
        return self.symbol_rate

    def set_symbol_rate(self, symbol_rate):
        self.symbol_rate = symbol_rate
        self.set_sps((self.samp_rate*1.0)/(self.symbol_rate*1.0))
        self.wxgui_scopesink2_0_0.set_sample_rate(self.symbol_rate)
        self.wxgui_scopesink2_0.set_sample_rate(self.symbol_rate)
        self.root_raised_cosine_filter_0.set_taps(firdes.root_raised_cosine(1, self.samp_rate, self.symbol_rate, 0.5, 361))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.set_sps((self.samp_rate*1.0)/(self.symbol_rate*1.0))
        self.wxgui_fftsink2_0.set_sample_rate(self.samp_rate)
        self.root_raised_cosine_filter_0.set_taps(firdes.root_raised_cosine(1, self.samp_rate, self.symbol_rate, 0.5, 361))
        self.blocks_throttle_0.set_sample_rate(self.samp_rate)

    def get_vgagain(self):
        return self.vgagain

    def set_vgagain(self, vgagain):
        self.vgagain = vgagain
        self._vgagain_slider.set_value(self.vgagain)
        self._vgagain_text_box.set_value(self.vgagain)

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

    def get_lnagain(self):
        return self.lnagain

    def set_lnagain(self, lnagain):
        self.lnagain = lnagain
        self._lnagain_slider.set_value(self.lnagain)
        self._lnagain_text_box.set_value(self.lnagain)

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


def main(top_block_cls=demod_tcp, options=None):

    tb = top_block_cls()
    tb.Start(True)
    tb.Wait()


if __name__ == '__main__':
    main()
