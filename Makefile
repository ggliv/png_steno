SRC_DIR:=src
INC_DIR:=inc
OBJ_DIR:=obj

CC:=gcc
CFLAGS:=-Wall -Wextra -pedantic -std=c11 -O3 -march=native -lm -iquote ${INC_DIR}

.PHONY: all clean dirs

all: encoder decoder

clean:
	rm -rf ${OBJ_DIR} decoder encoder

dirs:
	@mkdir -p ${SRC_DIR} ${INC_DIR} ${OBJ_DIR}

${OBJ_DIR}/%.o: ${SRC_DIR}/%.c | dirs
	${CC} ${CFLAGS} -c $< -o $@

decoder: ${OBJ_DIR}/decoder.o ${OBJ_DIR}/image.o ${OBJ_DIR}/huffman.o ${OBJ_DIR}/bit_stream.o ${OBJ_DIR}/crc.o
	${CC} ${CFLAGS} -o $@ $^

encoder: ${OBJ_DIR}/encoder.o ${OBJ_DIR}/image.o ${OBJ_DIR}/huffman.o ${OBJ_DIR}/bit_stream.o ${OBJ_DIR}/crc.o
	${CC} ${CFLAGS} -o $@ $^
