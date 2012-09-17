/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2012 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2012 - Daniel De Matteis
 * 
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

package com.retroarch;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

public class audio_android
{
     private static AudioTrack _track;
     private static int        _minSamples;
     private static float      _volume = AudioTrack.getMaxVolume();
     
     private static int _rate;
     private static int _bits;
     private static int _channels;
          
     
     private audio_android()
     {               
    	 
     }
          
          
     public static void pause()
     {
          if (_track != null && _track.getPlayState() != AudioTrack.PLAYSTATE_PAUSED)
        	  _track.pause();
     }


     public static void resume()
     {
          if (_track != null && _track.getPlayState() != AudioTrack.PLAYSTATE_PLAYING)       
               _track.play();
     }
     
     public int getMinSamples()
     {
          return _minSamples;
     }
     
     
     public static int write(short[] data, int size)
     {
          int retVal = 0;
          if (_track == null)
               return retVal;
          
          retVal = _track.write(data, 0, size);
          
          return retVal;
     }
          
                                       
     void set_volume(int vol)
     {
          final float min = AudioTrack.getMinVolume();
          final float max = AudioTrack.getMaxVolume();
          _volume = min + (max - min) * vol / 100;

          if (_track != null)
               _track.setStereoVolume(_volume, _volume);
     }               


     public static void free()
     {    
          if (_track != null)
          {
               _track.pause();
               _track.release();
               _track = null;
          }
     }


     public static boolean init(int rate, int bits, int channels)
     {                 
          _track = null;          
          _rate = rate;
          _bits = bits;
          _channels = channels;
          
          // generate format
          int format = (_bits == 16
                    ? AudioFormat.ENCODING_PCM_16BIT
                    : AudioFormat.ENCODING_PCM_8BIT);

          // determine channel config
          int channelConfig = (_channels == 2
                    ? AudioFormat.CHANNEL_OUT_STEREO
                    : AudioFormat.CHANNEL_OUT_MONO);

          int bufferSize = AudioTrack.getMinBufferSize(_rate, channelConfig,
                    format);
          
          _minSamples = bufferSize;
          
          try
          {
               _track = new AudioTrack(
                              AudioManager.STREAM_MUSIC,
                              _rate,
                              channelConfig,
                              format,
                              bufferSize,
                              AudioTrack.MODE_STREAM);

               if (_track.getState() == AudioTrack.STATE_UNINITIALIZED)
                    _track = null;

          }
          catch (IllegalArgumentException e)
          {
               _track = null;
               return false;
          }

          // set max volume
          _track.setStereoVolume(_volume, _volume);      
                   
          return true;
     }
    
              
     public static int getMaxBufferSize()
     {
          return _minSamples;
     }
}
