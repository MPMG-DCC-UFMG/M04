NAME = multidupehack
CXX = clang++ -Ofast -flto -DBOOST_MATH_OVERFLOW_ERROR_POLICY=ignore_error -Wall -Wextra -Weffc++ -std=c++11 -pedantic -Wno-overloaded-virtual -Wno-unused-parameter
# CXX = g++ -Ofast -flto -DBOOST_MATH_OVERFLOW_ERROR_POLICY=ignore_error -Wall -Wextra -Weffc++ -std=c++11 -pedantic -Wno-unused-parameter # with g++-9, the executable is ~10% slower than with clang++-9
# CXX = g++ -g -DBOOST_MATH_OVERFLOW_ERROR_POLICY=ignore_error -Wall -Wextra -Weffc++ -std=c++11 -pedantic -Wno-unused-parameter
EXTRA_CXXFLAGS = -lboost_program_options
HELP2MAN = help2man -n 'List every (closed) noise-tolerant n-set satisfying given constraints and/or optimizing given measures' -N
SRC = src/utilities src/measures src/core
DEPS = $(wildcard $(SRC:=/*.h)) Parameters.h Makefile
CODE = $(wildcard $(SRC:=/*.cpp))
OBJ = $(CODE:.cpp=.o)
ALL = $(DEPS) $(CODE) $(NAME).1.gz COPYING example INSTALL README

.PHONY: man install clean dist-gzip dist-bzip2 dist-xz dist
.SILENT: $(NAME) man install clean dist-gzip dist-bzip2 dist-xz dist

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $<

$(NAME): $(OBJ)
	$(CXX) -o $@ $^ $(EXTRA_CXXFLAGS)
	echo "$(NAME) built!"

man:
	$(HELP2MAN) ./$(NAME) | gzip > $(NAME).1.gz

install: $(NAME) $(NAME).1.gz
	mv $(NAME) /usr/bin
	cp $(NAME).1.gz /usr/share/man/man1/
	echo "$(NAME) installed!"

clean:
	rm -f $(patsubst %,%/*.o,$(SRC)) $(patsubst %,%/*~,$(SRC)) *~

dist-gzip:
	tar --format=posix --transform 's,^,$(NAME)/,' -czf $(NAME).tar.gz $(ALL)

dist-bzip2:
	tar --format=posix --transform 's,^,$(NAME)/,' -cjf $(NAME).tar.bz2 $(ALL)

dist-xz:
	tar --format=posix --transform 's,^,$(NAME)/,' -cJf $(NAME).tar.xz $(ALL)

dist: dist-bzip2
