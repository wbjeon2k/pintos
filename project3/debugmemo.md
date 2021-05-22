** Project 3 **

../../vm/frame.c:16:29: error: unknown type name ‘palloc_flags’
 inline bool palloc_chk_user(palloc_flags pf) {
                             ^
../../vm/frame.c:21:29: error: unknown type name ‘palloc_flags’
 inline bool palloc_chk_zero(palloc_flags pf) {
                             ^
../../vm/frame.c:26:10: warning: no previous prototype for ‘frmae_hf’ [-Wmissing            -prototypes]
 unsigned frmae_hf(const struct hash_elem* e, void* aux UNUSED) {
          ^
../../vm/frame.c:63:19: error: unknown type name ‘palloc_flags’
 void* frame_alloc(palloc_flags pf) {
                   ^
../../vm/frame.c: In function ‘frame_free’:
../../vm/frame.c:106:5: error: invalid type argument of ‘->’ (have ‘struct FTE’)
  tmp->PA = ptr;
     ^
../../vm/frame.c:109:39: error: invalid type argument of ‘->’ (have ‘struct FTE’
