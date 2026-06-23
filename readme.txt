This project has implementation of chained, non owning hash table and
doubly linked list, with node pool allocator and index based addressing.


# features to be implemented:
1. kvarena:
    - clear()
    - deref() (optional)
    - iterator # already achieved
    - deletion count # already achieved
2. kvht:
    - clear()
    - optional iterator
3. kvfile:
    - better encapsulation
    - code refractor
4. kvtbl:
    - better encapsulation
    - implm more completed wrappers around kvht, kvarena and kvfile
    - potentially building another separate lib (libkvtbl)
5. the whole project:
    - more testing (especially on kvfile)
    - cxx wrapper (RAII)
    - put struct typedefs in a separate header, with awareness of
    intrnl vs public struct separations
    - constent use of logging (_dbg_print vs _dbg_log_msg)