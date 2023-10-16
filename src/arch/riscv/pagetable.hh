/*
 * Copyright (c) 2002-2005 The Regents of The University of Michigan
 * Copyright (c) 2007 MIPS Technologies, Inc.
 * Copyright (c) 2020 Barkhausen Institut
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __ARCH_RISCV_PAGETABLE_H__
#define __ARCH_RISCV_PAGETABLE_H__

#include "arch/riscv/page_size.hh"
#include "base/bitunion.hh"
#include "base/logging.hh"
#include "base/trie.hh"
#include "base/types.hh"
#include "mem/port_proxy.hh"
#include "sim/serialize.hh"

namespace gem5
{

namespace RiscvISA {

BitUnion64(SATP)
    Bitfield<63, 60> mode;
    Bitfield<59, 44> asid;
    Bitfield<43, 0> ppn;
EndBitUnion(SATP)

enum AddrXlateMode
{
    BARE = 0,
    SV39 = 8,
    SV48 = 9,
};

// Sv39 paging
const Addr VADDR_BITS  = 39;
const Addr LEVEL_BITS  = 9;
const Addr LEVEL_MASK  = (1 << LEVEL_BITS) - 1;

BitUnion64(PTESv39)
    Bitfield<53, 10> ppn;
    Bitfield<53, 28> ppn2;
    Bitfield<27, 19> ppn1;
    Bitfield<18, 10> ppn0;
    Bitfield<7> d;
    Bitfield<6> a;
    Bitfield<5> g;
    Bitfield<4> u;
    Bitfield<3, 1> perm;
    Bitfield<3> x;
    Bitfield<2> w;
    Bitfield<1> r;
    Bitfield<0> v;
EndBitUnion(PTESv39)

template <int first, int last>
class HierarchySv39
{
public:
    Addr paddr()            { return pte.ppn << PageShift; }
    void paddr(Addr addr)   { pte.ppn = addr >> PageShift; }

    bool present()          { return pte.v; }
    void present(bool p)    { pte.v = p ? 1 : 0; }

    bool uncacheable()      { return true; }
    void uncacheable(bool u){ }

    bool readonly()         { return !pte.w; }
    void readonly(bool r)   { pte.w = r ? 0 : 1; }

    bool read()             { return pte.r; }
    void read(bool r)       { pte.r = r ? 1 : 0; }
    bool write()            { return pte.w; }
    void write(bool w)      { pte.w = w ? 1 : 0; }
    bool exec()             { return pte.x; }
    void exec(bool x)       { pte.x = x ? 1 : 0; }

    PTESv39 getPTE()        { return pte; }


    void
    read(PortProxy &p, Addr table, Addr vaddr)
    {
        entryAddr = table;
        entryAddr += bits(vaddr, first, last) * sizeof(PTESv39);
        pte = p.read<PTESv39>(entryAddr);
    }

    void
    reset(Addr _paddr, bool _present=true, bool _uncacheable=false,
        bool _readonly=false)
    {
        pte = 0;
        paddr(_paddr);
        present(_present);
        uncacheable(_uncacheable);
        readonly(_readonly);
        write(false);
        read(false);
        exec(false);
    }

    void
    reset_leaf(Addr _paddr, bool _executable, bool _present=true, bool _uncacheable=false,
        bool _readonly=false) {
        pte = 0;
        paddr(_paddr);
        present(_present);
        uncacheable(_uncacheable);
        read(true);
        write(true);
        exec(_executable);
    }

    void
    write(PortProxy &p)
    {
        p.write(entryAddr, pte);
    }

    static int
    tableSize()
    {
        return 1 << ((first - last) + 4 - PageShift);
    }

protected:
    PTESv39 pte;
    Addr entryAddr;
};

struct TlbEntry;
typedef Trie<Addr, TlbEntry> TlbEntryTrie;

struct TlbEntry : public Serializable
{
    // The base of the physical page.
    Addr paddr;

    // The beginning of the virtual page this entry maps.
    Addr vaddr;
    // The size of the page this represents, in address bits.
    unsigned logBytes;

    uint16_t asid;

    PTESv39 pte;

    TlbEntryTrie::Handle trieHandle;

    // A sequence number to keep track of LRU.
    uint64_t lruSeq;

    TlbEntry()
        : paddr(0), vaddr(0), logBytes(0), pte(), lruSeq(0)
    {}

    // Return the page size in bytes
    Addr size() const
    {
        return (static_cast<Addr>(1) << logBytes);
    }

    void serialize(CheckpointOut &cp) const override;
    void unserialize(CheckpointIn &cp) override;
};

} // namespace RiscvISA
} // namespace gem5

#endif // __ARCH_RISCV_PAGETABLE_H__
