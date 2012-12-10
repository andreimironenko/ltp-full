#!/bin/sh
#
# Generate generic POSIX compliant Makefiles.
#
# This means that there's a lot of unnecessary text (when using BSD or GNU
# make, as I'm sure there are in other variants), and a lack of modularity,
# but as long as you follow the criterion set in locate-test, then the
# end-result for modifying and/or adding tests can be achieved by merely
# rerunning this script.
#
# This script will remain around until (hopefully someday) POSIX make
# becomes less braindead.
#
# See COPYING for more details.
#
# Garrett Cooper, June 2010
#

generate_locate_test_makefile() {

	local maketype=$1; shift

	echo "Generating $maketype Makefiles"

	locate-test --$maketype | sed -e 's,^./,,g' > make-gen.$maketype

	generate_makefiles make-gen.$maketype $*

	rm -f make-gen.$maketype

}

generate_makefile() {

	local make_target_prereq_cache=
	local prereq_cache=
	local tests=

	local makefile=$1
	local prereq_dir=$2
	local compiler_args=$3
	shift 3

	prereq_cache=$*

	# Add all source files to $make_target_prereq_cache.
	for prereq in $prereq_cache; do 
		# Stuff that needs to be tested.
		if echo "$prereq" | grep -Eq '\.(run-test|sh)'; then
			if [ "$tests" != "" ]; then
				tests="$tests "
			fi
			tests="$tests$prereq"
		fi
		# Stuff that needs to be compiled.
		case "$prereq" in
		*.sh)
			# Catch.
			;;
		*)
			if [ "$make_target_prereq_cache" != "" ]; then
				make_target_prereq_cache="$make_target_prereq_cache "
			fi
			make_target_prereq_cache="$make_target_prereq_cache$prereq"
			;;
		esac
	done

	if [ ! -f "$makefile.1" ]; then

		cat > "$makefile.1" <<EOF
#
# Automatically generated by `basename "$0"` -- DO NOT EDIT.
#
# Restrictions for `basename "$0"` apply to this file. See COPYING for
# more details.
#
# $AUTHORDATE
#

# Path variables.
top_srcdir?=		`echo "$prereq_dir" | awk '{ gsub(/[^\/]+/, "..", $0); print }'`
subdir=			$prereq_cache_dir
srcdir=			\$(top_srcdir)/\$(subdir)

prefix?=		$PREFIX
exec_prefix?=		\$(prefix)
INSTALL_DIR=		\$(DESTDIR)/\$(exec_prefix)/\$(subdir)
LOGFILE?=		logfile

# Build variables
CFLAGS+=		-I\$(top_srcdir)/include

# XXX: for testfrmw.c -- needs to be moved into a library.
CFLAGS+=		-I\$(srcdir)

EOF

		if [ -f "$GLOBAL_BOILERPLATE" ]; then
			cat >> "$makefile.1" <<EOF
# Top-level make definitions
`cat $GLOBAL_BOILERPLATE`
EOF
		fi

		cat >> "$makefile.1" <<EOF
# Submake make definitions.
EOF

		for var in CFLAGS LDFLAGS LDLIBS; do
			if [ -f "${TOP_SRCDIR}/$var" ]; then
				cat >> "$makefile.1" <<EOF
${var}+=		`cat "${prereq_cache_dir}/${var}" 2>/dev/null`
EOF
			fi
		done

		# Whitespace
		echo "" >> "$makefile.1"

	fi

	cat >> "$makefile.2" <<EOF
INSTALL_TARGETS+=	${tests}
MAKE_TARGETS+=		${make_target_prereq_cache}

EOF

	if [ ! -f "$makefile.3" ]; then

		cat > "$makefile.3" <<EOF
all: \$(MAKE_TARGETS)
	@if [ -d speculative ]; then \$(MAKE) -C speculative all; fi

clean:
	rm -f \$(MAKE_TARGETS) logfile* run.sh *.core
	@if [ -d speculative ]; then \$(MAKE) -C speculative clean; fi

install: \$(INSTALL_DIR) run.sh
	set -e; for file in \$(INSTALL_TARGETS) run.sh; do	\\
		if [ -f "\$\$file" ] ; then			\\
			install -m 00755 \$\$file		\\
				\$(INSTALL_DIR)/\$\$file;	\\
		fi;						\\
	done
	@if [ -d speculative ]; then \$(MAKE) -C speculative install; fi

test: run.sh
	@./run.sh

\$(INSTALL_DIR):
	mkdir -p \$@

EOF

	fi

	if ! grep -q '^run.sh' "$makefile.3"; then
		cat >> "$makefile.3" <<EOF
run.sh:
	@echo '#/bin/sh' > \$@
	@echo "\$(top_srcdir)/bin/run-tests.sh \$(subdir) \$(INSTALL_TARGETS)" >> \$@
	@chmod +x run.sh

EOF
	fi

	# Produce _awesome_ target rules for everything that needs it.
	for prereq in ${make_target_prereq_cache}; do

		test_name="$prereq"
		if [ "$suffix" != "" ]; then
			test_name=`echo "$test_name" | sed -e "s,$suffix,,"`
		fi
		c_file="$test_name.c"

		case "$suffix" in
		.run-test)
			grep -q 'main' "$prereq_dir/$c_file" || echo >&2 "$prereq_dir/$c_file should be test."
			;;
		.test)
			grep -q 'main' "$prereq_dir/$c_file" && echo >&2 "$prereq_dir/$c_file should be run-test."
			;;
		esac

		COMPILE_STR="\$(CC) $compiler_args \$(CFLAGS) \$(LDFLAGS) -o \$@ \$(srcdir)/$c_file \$(LDLIBS)"

		cat >> "$makefile.3" <<EOF
$prereq: \$(srcdir)/$c_file
	@if $COMPILE_STR >logfile.\$\$\$\$ 2>&1; then \\
		 echo "\$(subdir)/$test_name compile PASSED"; \\
		 echo "\$(subdir)/$test_name compile PASSED" >> \$(LOGFILE); \\
	else \\
		 echo "\$(subdir)/$test_name compile FAILED; SKIPPING"; \\
		(echo "\$(subdir)/$test_name compile FAILED; SKIPPING"; cat logfile.\$\$\$\$) >> \$(LOGFILE); \\
	fi; \\
	rm -f logfile.\$\$\$\$

EOF

	done

}

generate_makefiles() {

	local prereq_cache=

	local make_gen_list=$1
	local suffix=$2
	local compiler_args="$3"

	while read filename; do

		prereq_dir=`dirname "$filename"`

		# First run.
		if [ "$prereq_cache_dir" = "" ] ; then
			prereq_cache_dir="$prereq_dir"
		elif [ "$prereq_cache_dir" != "$prereq_dir" ]; then

			generate_makefile "$prereq_cache_dir/Makefile" "$prereq_cache_dir" "$compiler_args" $prereq_cache

			# Prep for the next round..
			prereq_cache=
			prereq_cache_dir="$prereq_dir"

		fi

		# Cache the entries to punt out all of the data at
		# once for a single Makefile.
		if [ "$prereq_cache" != "" ] ; then
			prereq_cache="$prereq_cache "
		fi
		prereq_cache="$prereq_cache"`basename "$filename" | sed "s,.c\$,$suffix,g"`

	done < $make_gen_list

	# Dump the last Makefile data cached up.
	generate_makefile "$prereq_cache_dir/Makefile" $prereq_cache_dir "$compiler_args" $prereq_cache

}

export PATH="$PATH:`dirname "$0"`"

AUTHORDATE=`grep "Garrett Cooper" "$0" | head -n 1 | sed 's,# *,,'`
PREFIX=`print-prefix.sh`
EXEC_PREFIX="${PREFIX}/bin"
TOP_SRCDIR=${TOP_SRCDIR:=`dirname "$0"`/..}

GLOBAL_BOILERPLATE="${TOP_SRCDIR}/.global_boilerplate"

rm -f "$GLOBAL_BOILERPLATE"

for var in CFLAGS LDFLAGS LDLIBS; do
	if [ -f "$TOP_SRCDIR/$var" ]; then
		cat >> "$GLOBAL_BOILERPLATE" <<EOF
$var+=		`cat "$TOP_SRCDIR/$var"`
EOF
	fi
done

# For the generic cases.
generate_locate_test_makefile buildonly '.test' '-c'
generate_locate_test_makefile runnable '.run-test'
generate_locate_test_makefile test-tools ''

rm -f "$GLOBAL_BOILERPLATE"

find . -name Makefile.1 -exec dirname {} \; | while read dir; do
	if [ -f "$dir/Makefile.2" ]; then
		cat $dir/Makefile.1 $dir/Makefile.2 $dir/Makefile.3 > $dir/Makefile
	fi
	rm $dir/Makefile.1 $dir/Makefile.2 $dir/Makefile.3
done
