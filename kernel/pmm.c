#include <strings.h>
#include "limine.h"
#include "franklin/mmu.h"
#include "franklin/defs.h"
#include "franklin/spinlock.h"




void togglepage(int page) {
  bitmap[page / 64] ^= (1ULL << (page % 64));
};
uint8_t isfree(int page) {
    return (((bitmap[page / 64] & (1ULL << (page % 64)))) == 0);
}


void *palloc(int size) {
    if (size >= MAXPG)
        panic("panic: palloc, size too big");
    acquire(&spinlock);
    int page = 0, p = 0;
    int x = 0, i = 0;

    while (1) {
        if (page >= MAXPG)
            panic("panic: out of pages\n");
        // if true all pages are inuse
        if (bitmap[page / 64] == INT64_MAX) {
            page++;
            continue;
        }
        if (isfree(page)) {

            // if value is zero all 64 pages are free
            if (bitmap[page / 64] == 0)
                goto setpages;

            // check that there are 'size' amount of pages free 
            for (p = page; p < (size + page); ++p) {
                if (!isfree(p))
                    break;
            }

            // mark the pages as used
            if (p == size + page) {
                setpages:
                for (p = page; p < (size + page); ++p) 
                    togglepage(p);
                goto releaselock; 
            }
        };
        page++;
    };
    releaselock:
    release(&spinlock);
    return (void*)((uintptr_t)(page * PGSIZE));
}   


void *pallocaddr(int size, uint64_t paddr) {
    acquire(&spinlock);
    int pfn = paddr / PGSIZE;
    for (int i = pfn; i < pfn + size; ++i) {
        if (!isfree(i))
            panic("panic: pallocaddr, page is not free\n");
        togglepage(i);
    };
    release(&spinlock);
    return (void*)paddr;
}


void freepg(uintptr_t addr, int length) {
    int page = addr / PGSIZE;
    acquire(&spinlock);
    do {
        togglepage(page);
    } while(--length && page++);
    release(&spinlock);
}

void initbmap(struct limine_memmap_response *memmap) {
    uint64_t bitmapsz = getmemsz(memmap);
    struct limine_memmap_entry *entry = getentry(memmap, bitmapsz);

    bitmap = (uint64_t)HHDM_OFFSET + entry->base;
    for (int i = 0; i < bitmapsz; ++i)
        togglepage(entry->base / PGSIZE + i);

    entry->base += bitmapsz;
    entry->length -= bitmapsz;
    for (int i = 0; i < memmap->entry_count; ++i) {
        if (memmap->entries[i]->type != 0)
            setentry(memmap->entries[i]);
    }
}


void setentry(struct limine_memmap_entry *entry) {
    uint_t page = entry->base / PGSIZE;
    acquire(&spinlock);
    for (int i = 0; i < (entry->length / PGSIZE); i++, page++) {
        togglepage(page);
    }
    release(&spinlock);
}


uint64_t getmemsz(struct limine_memmap_response* memmap) {
    uint64_t size = memmap->entries[14]->base + memmap->entries[14]->length;

    MAXPG = size / PGSIZE;
    size /= PGSIZE;
    size /= 8;
    size /= PGSIZE;
    return size;
}

struct limine_memmap_entry* getentry(struct limine_memmap_response *memmap, uint64_t mapsize) {

    int i = 0;
    while (1) {
        if (memmap->entries[i]->type == 0 && memmap->entries[i]->length > mapsize)
            return memmap->entries[i];
        i++;
    }
    return 0;
}
