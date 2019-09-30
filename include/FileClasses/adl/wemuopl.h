/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2008 Simon Peter, <dn.tlp@gmx.net>, et al.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * kemuopl.h - Emulated OPL using the DOSBox OPL3 emulator
 */

#ifndef H_ADPLUG_WEMUOPL
#define H_ADPLUG_WEMUOPL

#include "opl.h"
#include <FileClasses/adl/nukedopl3.h>
extern "C" {
#include "woodyopl.h"
}

class CWemuopl: public Copl
{
public:
  CWemuopl(int rate, bool usestereo)
    : stereo(usestereo)
    {
      OPL3_Reset(&opl, rate);
      currType = TYPE_OPL2;
    }

  void update(short *buf, int samples) override
  {
      OPL3_GenerateStream(&opl, buf, samples);
      for(int i = 0; i < samples * 2; i++)
          buf[i] *= 4;
  }

  // template methods
  void write(int reg, int val) override
  {
      if(currChip != 0)
          return;

      OPL3_WriteRegBuffered(&opl, reg, val);
  }

  void init() override {}

private:
  bool      stereo;
  opl3_chip opl;
};

#endif
