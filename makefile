PREFIX=
DESTDIR=/usr
CC=gcc
CFLAGS+= -shared -fPIC
LIB_NAME=$(shell pwd | rev | cut -d'/' -f1 | rev )
LIB_OBJ=lib${LIB_NAME}.so
LIB_HEADERS=${LIB_NAME}.h

MAN_SECTION=3
MAN_SRC=$(shell find .man 2>/dev/null| grep -E "${LIB_NAME}.*\.${MAN_SECTION}\.gz" | rev | cut -d'/' -f1 | rev )

help:
	@echo "available cmd are make help|install|uninstall|clean"
	@echo "available args:"
	@echo "    LIB_NAME current \"${LIB_NAME}\""
	@echo "    CC current \"${CC}\""
	@echo "    DESTDIR current \"${DESTDIR}\""
	@echo "    PREFIX current \"${PREFIX}\""

lib%.so: %.c
	${CC} -o $@ $^ ${CFLAGS}

install: ${LIB_OBJ}
	cp ${LIB_HEADERS} ${DESTDIR}/${PREFIX}/include
	install ${LIB_OBJ} ${DESTDIR}/${PREFIX}/lib
	@for path in ${MAN_SRC} ; do \
		echo add $$path to man ; \
		mkdir -p /usr/share/man/man${MAN_SECTION} ; \
		install .man/$$path /usr/share/man/man${MAN_SECTION} ; \
	done

uninstall:
	@for header in ${LIB_HEADERS} ; do \
		echo rm ${DESTDIR}/${PREFIX}/include/$${header} ; \
		rm ${DESTDIR}/${PREFIX}/include/$${header} ; \
	done
	@for obj in ${LIB_OBJ} ; do \
		echo rm ${DESTDIR}/${PREFIX}/lib/$${obj} ; \
		rm ${DESTDIR}/${PREFIX}/lib/$${obj} ; \
	done
	@for path in ${MAN_SRC} ; do \
		echo rm $${path} form man ; \
		rm -f /usr/share/man/man${MAN_SECTION}/$${path} ; \
	done

clean:
	rm ${LIB_OBJ}
