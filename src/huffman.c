#include "bit_stream.h"
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if CHAR_BIT != 8
#error Must compile on a machine where CHAR_BIT == 8
#endif

#define REDUCED_ASCII_LEN 99

unsigned char map_reduced_ascii(unsigned char original) {
  if (original == '\0') {
    return 0x00;
  } else if (original == '\t') {
    return 0x01;
  } else if (original == '\n') {
    return 0x02;
  } else if (original == '\r') {
    return 0x03;
  } else if (0x20 <= original && original <= 0x7E) {
    return original - 0x1C;
  } else {
    fprintf(stderr, "ERROR: Cannot map character 0x%X.\n", original);
    exit(EXIT_FAILURE);
  }
}

unsigned char unmap_reduced_ascii(unsigned char mapped) {
  if (mapped == 0x00) {
    return '\0';
  } else if (mapped == 0x01) {
    return '\t';
  } else if (mapped == 0x02) {
    return '\n';
  } else if (mapped == 0x03) {
    return '\r';
  } else if (0x04 <= mapped && mapped <= 0x7E) {
    return mapped + 0x1C;
  } else {
    fprintf(stderr, "ERROR: Cannot unmap character 0x%X.\n", mapped);
    exit(EXIT_FAILURE);
  }
}

// heap code is referenced from CLRS Ch. 6

struct AsciiNode {
  unsigned char symbol;
  unsigned freq;
  struct AsciiNode *left, *right;
};

struct AsciiHeap {
  struct AsciiNode *heap[REDUCED_ASCII_LEN];
  size_t len;
};

bool is_node_leaf(const struct AsciiNode *const node) {
  return (node->right == NULL) && (node->left == NULL);
}

// i is one-based (e.g. if min-heapifying from the root, pass 1)
void min_heapify(struct AsciiHeap *ascii_heap, size_t i) {
  struct AsciiNode **heap = ascii_heap->heap;
  size_t len = ascii_heap->len;
  size_t l = 2 * i, r = 2 * i + 1, smallest = i;
  if (l <= len && heap[l - 1]->freq < heap[i - 1]->freq) {
    smallest = l;
  }
  if (r <= len && heap[r - 1]->freq < heap[smallest - 1]->freq) {
    smallest = r;
  }
  if (smallest != i) {
    struct AsciiNode *hold = heap[i - 1];
    heap[i - 1] = heap[smallest - 1];
    heap[smallest - 1] = hold;
    min_heapify(ascii_heap, smallest);
  }
}

void build_min_heap(struct AsciiHeap *ascii_heap) {
  for (size_t i = ascii_heap->len / 2; i > 0; i--) {
    min_heapify(ascii_heap, i);
  }
}

// fails assertion on heap underflow, check before calling
struct AsciiNode *extract_min(struct AsciiHeap *ascii_heap) {
  assert(ascii_heap->len != 0);

  struct AsciiNode *min = ascii_heap->heap[0];
  ascii_heap->heap[0] = ascii_heap->heap[ascii_heap->len - 1];
  ascii_heap->len -= 1;
  min_heapify(ascii_heap, 1);
  return min;
}

struct SymbolAndCode {
  unsigned char symbol;
  bool code[REDUCED_ASCII_LEN - 1];
  size_t code_len;
};

struct HuffmanTable {
  struct SymbolAndCode table[REDUCED_ASCII_LEN];
};

void tabulate_huffman_tree(struct HuffmanTable *table,
                           const struct AsciiNode *const root, int code_len) {
  if (is_node_leaf(root)) {
    struct SymbolAndCode *s = &table->table[root->symbol];
    s->symbol = root->symbol;
    memset(s->code, false, sizeof(s->code));
    s->code_len = code_len;
  } else {
    tabulate_huffman_tree(table, root->left, code_len + 1);
    tabulate_huffman_tree(table, root->right, code_len + 1);
  }
}

void free_huffman_tree(struct AsciiNode *root) {
  if (!is_node_leaf(root)) {
    free_huffman_tree(root->left);
    free_huffman_tree(root->right);
  }
  free(root);
}

int canonical_comparator(const void *a_p, const void *b_p) {
  const struct SymbolAndCode *a = a_p, *b = b_p;
  if (a->code_len == 0) {
    return 1;
  } else if (b->code_len == 0) {
    return -1;
  }

  if (a->code_len != b->code_len) {
    return a->code_len - b->code_len;
  } else {
    return (int)a->symbol - (int)b->symbol;
  }
}

void bit_arr_shl(bool *arr, size_t len) {
  assert(len > 0);
  bool carry = false;
  for (size_t i = 0; i < len; i++) {
    bool hold = arr[i];
    arr[i] = carry;
    carry = hold;
  }
}

void bit_arr_inc(bool *arr, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (arr[i]) {
      arr[i] = false;
    } else {
      arr[i] = true;
      break;
    }
  }
}

void generate_canonical_codes(struct HuffmanTable *table) {
  qsort(table->table, REDUCED_ASCII_LEN, sizeof(struct SymbolAndCode),
        &canonical_comparator);
  bool bit_arr[REDUCED_ASCII_LEN - 1];
  for (size_t i = 0; i < REDUCED_ASCII_LEN - 1; i++) {
    bit_arr[i] = false;
    table->table[0].code[i] = false;
  }
  size_t bit_arr_len = table->table[0].code_len;
  for (size_t i = 1; i < REDUCED_ASCII_LEN; i++) {
    struct SymbolAndCode *snc = &table->table[i];
    if (snc->code_len == 0) {
      break;
    }
    bit_arr_inc(bit_arr, REDUCED_ASCII_LEN - 1);
    while (bit_arr_len < snc->code_len) {
      bit_arr_shl(bit_arr, REDUCED_ASCII_LEN - 1);
      bit_arr_len += 1;
    }
    memcpy(snc->code, bit_arr, sizeof(bit_arr));
  }
}

void bs_insert_code_len(struct BitStream *bs, unsigned char len) {
  for (int i = 6; i >= 0; i--) {
    unsigned char bit = (len >> i) & 1;
    bs_write_bit(bs, bit);
  }
}

unsigned char bs_extract_code_len(struct BitStream *bs) {
  unsigned char len = 0;
  for (int i = 6; i >= 0; i--) {
    bool bit = !!bs_read_bit(bs);
    len |= bit << i;
  }
  return len;
}

void bs_insert_code(struct BitStream *bs, struct SymbolAndCode *snc) {
  size_t code_len = snc->code_len;
  while (code_len > 0) {
    bs_write_bit(bs, snc->code[code_len - 1]);
    code_len--;
  }
}

struct BitStream encode_message(struct HuffmanTable *table,
                                const char *message) {
  struct SymbolAndCode *lut[REDUCED_ASCII_LEN] = {0};
  for (size_t i = 0; i < REDUCED_ASCII_LEN; i++) {
    struct SymbolAndCode *snc = &table->table[i];
    if (snc->code_len == 0) {
      break;
    }
    lut[snc->symbol] = snc;
  }

  struct BitStream bs = {
      /*
       * TODO I'm /pretty/ sure we can just do strlen(msg) + RED_ASCII_LEN, but
       * I'm not entirely convinced and don't want to take the time to prove
       * it. we'll just double the length of the message here to be safe.
       */
      .data = calloc(strlen(message) * 2 + REDUCED_ASCII_LEN, 1),
      .byte_offset = 0,
      .bit_offset = 0,
  };

  for (int i = 0; i < REDUCED_ASCII_LEN; i++) {
    if (lut[i] != NULL) {
      bs_insert_code_len(&bs, lut[i]->code_len);
    } else {
      bs_insert_code_len(&bs, 0);
    }
  }

  for (size_t i = 0; message[i] != '\0'; i++) {
    struct SymbolAndCode *this_c = lut[map_reduced_ascii(message[i])];
    bs_insert_code(&bs, this_c);
  }
  bs_insert_code(&bs, lut[map_reduced_ascii('\0')]);
  bs.data_len = bs.byte_offset + !!bs.bit_offset + 1;
  bs.data = realloc(bs.data, bs.data_len);
  return bs;
}

struct BitStream huffman_encode(const char *const message) {
  struct AsciiHeap ascii_heap = {.len = REDUCED_ASCII_LEN};
  for (unsigned char c = 0; c < REDUCED_ASCII_LEN; c++) {
    struct AsciiNode *node = malloc(sizeof(struct AsciiNode));

    node->symbol = c;
    node->freq = 0;
    node->left = node->right = NULL;

    ascii_heap.heap[c] = node;
  }

  // count the frequency of each char in message
  for (const char *c_p = message; *c_p != '\0'; c_p++) {
    unsigned char c = map_reduced_ascii(*c_p);
    ascii_heap.heap[c]->freq += 1;
  }

  // we'll use one '\0' later to indicate end of message
  ascii_heap.heap[map_reduced_ascii('\0')]->freq = 1;

  build_min_heap(&ascii_heap);

  // remove symbos from the heap that weren't present in the message
  while (ascii_heap.len > 0 && ascii_heap.heap[0]->freq == 0) {
    free(extract_min(&ascii_heap));
  }

  while (ascii_heap.len > 1) {
    struct AsciiNode *min = extract_min(&ascii_heap);
    struct AsciiNode *next_min = ascii_heap.heap[0];
    struct AsciiNode *fork = malloc(sizeof(struct AsciiNode));
    fork->freq = min->freq + next_min->freq;
    fork->left = min;
    fork->right = next_min;
    ascii_heap.heap[0] = fork;
    min_heapify(&ascii_heap, 1);
  }

  struct HuffmanTable tab = {0};
  tabulate_huffman_tree(&tab, ascii_heap.heap[0], 0);
  free_huffman_tree(ascii_heap.heap[0]);
  generate_canonical_codes(&tab);

  return encode_message(&tab, message);
}

struct AsciiNode *detabulate_huffman_tree(struct HuffmanTable *tab) {
  struct AsciiNode *root = calloc(1, sizeof(struct AsciiNode));
  for (unsigned char i = 0; i < REDUCED_ASCII_LEN; i++) {
    struct SymbolAndCode *snc = &tab->table[i];
    if (snc->code_len == 0) {
      continue;
    }
    struct AsciiNode *descend = root;
    for (int bit = snc->code_len - 1; bit >= 0; bit--) {
      if (snc->code[bit]) {
        if (descend->right == NULL) {
          descend->right = calloc(1, sizeof(struct AsciiNode));
        }
        descend = descend->right;
      } else {
        if (descend->left == NULL) {
          descend->left = calloc(1, sizeof(struct AsciiNode));
        }
        descend = descend->left;
      }
    }
    descend->symbol = snc->symbol;
    descend->left = NULL;
    descend->right = NULL;
  }
  return root;
}

void huffman_decode(struct BitStream *bs, FILE *writeback) {
  struct HuffmanTable tab = {0};
  bs->byte_offset = bs->bit_offset = 0;
  for (unsigned char i = 0; i < REDUCED_ASCII_LEN; i++) {
    tab.table[i].symbol = i;
    tab.table[i].code_len = bs_extract_code_len(bs);
  }

  generate_canonical_codes(&tab);

  struct AsciiNode *root = detabulate_huffman_tree(&tab);

  unsigned char last_decoded = 1;
  struct AsciiNode *descend = root;
  // all valid messages end with a '\0', which we break on in the loop
  while (true) {
    bool l = bs_read_bit(bs);
    if (l) {
      descend = descend->right;
    } else {
      descend = descend->left;
    }
    if (is_node_leaf(descend)) {
      last_decoded = unmap_reduced_ascii(descend->symbol);
      if (last_decoded == '\0') {
        break;
      }
      fprintf(writeback, "%c", last_decoded);
      descend = root;
    }
  }

  bs->data_len = bs->byte_offset + !!bs->bit_offset + 1;

  free_huffman_tree(root);
}
