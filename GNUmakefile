#
# Build httpd
#
include autoconf.mk

DESTDIR		=

OPTIMFLAGS     +=
WARNFLAGS      +=
DEFS	       += -DDEBUG
CPPFLAGS       += -Ispirit
CXXFLAGS       +=
LDFLAGS	       +=

OBJS	       += main.o log.o config.o HTTPParser.o rh-construction.o \
		  rh-copy-file.o rh-errors.o rh-get-request-body.o     \
		  rh-get-request-header.o rh-get-request-line.o        \
		  rh-readable.o rh-setup-reply.o rh-terminate.o        \
		  rh-timeouts.o rh-writable.o rh-log-access.o          \
		  rh-write-remaining-data.o rh-persistent-connections.o
LIBOBJS	       += libscheduler/scheduler.o
LIBS	       +=

.SUFFIXES:
.SUFFIXES:	.cc .o

.cc.o:
	$(CXX) $(CPPFLAGS) $(DEFS) $(CXXFLAGS) $(WARNFLAGS) $(OPTIMFLAGS) -c $< -o $@

all:		httpd

httpd:		$(OBJS) $(LIBOBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LIBOBJS) $(LIBS) -o $@

config.o:	config.cc
	$(CXX) -DDOCUMENT_ROOT=\"/home/simons/projects/httpd\"              \
	       -DLOGFILE=\"/home/simons/projects/httpd/logfiles/%s-access\" \
		$(CPPFLAGS) $(DEFS) $(CXXFLAGS) $(WARNFLAGS) $(OPTIMFLAGS) -c $< -o $@

configure:	configure.ac
	autoconf

autoconf.mk:	autoconf.mk.in configure
	${HTTPD_AC_VARS} $(SHELL) configure ${HTTPD_AC_ARGS}

install:	httpd
	$(MKDIR) -p $(DESTDIR)$(bindir)
	$(INSTALL) -c -m 555 httpd $(DESTDIR)$(bindir)/

clean::
	@find . -name '*.rpo' -exec rm -f {} \;
	@find . -name '*.a'   -exec rm -f {} \;
	@find . -name '*.o'   -exec rm -f {} \;
	@find . -name '*.so'  -exec rm -f {} \;
	@rm -f httpd
	@echo All dependent files have been removed.

distclean:: clean
	@rm -f autoconf.mk config.log config.status
	@rm -rf spirit/Makefile spirit/config.log spirit/config.status
	@rm -rf spirit/spirit.pc spirit/boost/Makefile
	@rm -rf spirit/boost/config/Makefile
	@rm -rf spirit/boost/config/compiler/Makefile
	@rm -rf spirit/boost/config/platform/Makefile
	@rm -rf spirit/boost/config/stdlib/Makefile
	@rm -rf spirit/boost/detail/Makefile
	@rm -rf spirit/boost/phoenix/Makefile
	@rm -rf spirit/boost/spirit/Makefile
	@rm -rf spirit/boost/spirit/MSVC/impl/Makefile
	@rm -rf spirit/boost/spirit/attr/Makefile
	@rm -rf spirit/boost/spirit/attr/impl/Makefile
	@rm -rf spirit/boost/spirit/core/Makefile
	@rm -rf spirit/boost/spirit/core/impl/Makefile
	@rm -rf spirit/boost/spirit/rule/Makefile
	@rm -rf spirit/boost/spirit/rule/impl/Makefile
	@rm -rf spirit/boost/spirit/tree/Makefile
	@rm -rf spirit/boost/spirit/tree/impl/Makefile
	@rm -rf spirit/boost/spirit/utility/Makefile
	@rm -rf spirit/boost/spirit/utility/impl/Makefile
	@rm -rf spirit/boost/tuple/Makefile
	@rm -rf spirit/boost/tuple/detail/Makefile
	@rm -rf spirit/boost/type_traits/Makefile spirit/libs/Makefile
	@rm -rf spirit/libs/build/Makefile
	@rm -rf spirit/libs/build/ICL/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/C/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/Calc1/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/Calc2/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/Calc3/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/Calc4/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/CalcDebug/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/ChSet/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/CharActions/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/CharParam/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/ExceptionTest/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/IterParam/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/Micro/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/NumericActions/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/Pascal/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/RegExpr/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/Roman/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/StringExtract/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/StringParam/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/Symbols/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/XML/FileXml
	@rm -rf spirit/libs/build/ICL/EXAMPLE/XML/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/attributed_rules/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/calc5/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/calc_ast/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/closure/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/comments/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/cpp/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/grammar/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/grammar_capsule/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/list_parser/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/parameters/Makefile
	@rm -rf spirit/libs/build/ICL/EXAMPLE/sexpr/Makefile
	@rm -rf spirit/libs/build/MSVC/Makefile
	@rm -rf spirit/libs/build/MSVC/C/Makefile
	@rm -rf spirit/libs/build/MSVC/C/c_grammar
	@rm -rf spirit/libs/build/MSVC/Calc1/Makefile
	@rm -rf spirit/libs/build/MSVC/Calc2/Makefile
	@rm -rf spirit/libs/build/MSVC/Calc3/Makefile
	@rm -rf spirit/libs/build/MSVC/Calc4/Makefile
	@rm -rf spirit/libs/build/MSVC/Pascal/Makefile
	@rm -rf spirit/libs/build/MSVC/Spirit/Makefile
	@rm -rf spirit/libs/build/MSVC/XML/Makefile
	@rm -rf spirit/libs/build/MSVC/XML/Xml
	@rm -rf spirit/libs/build/MSVC/XML/simplexml
	@rm -rf spirit/libs/build/MSVC/ast_xml/Makefile
	@rm -rf spirit/libs/build/MSVC/calc_debug/Makefile
	@rm -rf spirit/libs/build/MSVC/char_actions/Makefile
	@rm -rf spirit/libs/build/MSVC/char_param/Makefile
	@rm -rf spirit/libs/build/MSVC/chset/Makefile
	@rm -rf spirit/libs/build/MSVC/closure/Makefile
	@rm -rf spirit/libs/build/MSVC/comments/Makefile
	@rm -rf spirit/libs/build/MSVC/exception_test/Makefile
	@rm -rf spirit/libs/build/MSVC/grammar/Makefile
	@rm -rf spirit/libs/build/MSVC/grammar_capsule/Makefile
	@rm -rf spirit/libs/build/MSVC/iter_param/Makefile
	@rm -rf spirit/libs/build/MSVC/list_parser/Makefile
	@rm -rf spirit/libs/build/MSVC/micro/Makefile
	@rm -rf spirit/libs/build/MSVC/numeric_actions/Makefile
	@rm -rf spirit/libs/build/MSVC/parameters/Makefile
	@rm -rf spirit/libs/build/MSVC/polynomial/Makefile
	@rm -rf spirit/libs/build/MSVC/regexpr/Makefile
	@rm -rf spirit/libs/build/MSVC/rfc821/Makefile
	@rm -rf spirit/libs/build/MSVC/roman/Makefile
	@rm -rf spirit/libs/build/MSVC/string_extract/Makefile
	@rm -rf spirit/libs/build/MSVC/string_param/Makefile
	@rm -rf spirit/libs/build/MSVC/symbols/Makefile
	@rm -rf spirit/libs/doc/Makefile spirit/libs/doc/theme/Makefile
	@rm -rf spirit/libs/example/.deps spirit/libs/example/Makefile
	@rm -rf spirit/libs/example/c/.deps
	@rm -rf spirit/libs/example/c/Makefile
	@rm -rf spirit/libs/example/c/test_files/Makefile
	@rm -rf spirit/libs/example/calc/.deps
	@rm -rf spirit/libs/example/calc/Makefile
	@rm -rf spirit/libs/example/cpp/.deps
	@rm -rf spirit/libs/example/cpp/Makefile
	@rm -rf spirit/libs/example/cpp/doc/Makefile
	@rm -rf spirit/libs/example/cpp/doc/theme/Makefile
	@rm -rf spirit/libs/example/pascal/.deps
	@rm -rf spirit/libs/example/pascal/Makefile
	@rm -rf spirit/libs/example/pascal/test_files/Makefile
	@rm -rf spirit/libs/example/phoenix/.deps
	@rm -rf spirit/libs/example/phoenix/Makefile
	@rm -rf spirit/libs/example/slex/.deps
	@rm -rf spirit/libs/example/slex/Makefile
	@rm -rf spirit/libs/example/slex/testfiles/Makefile
	@rm -rf spirit/libs/example/xml/.deps
	@rm -rf spirit/libs/example/xml/Makefile spirit/libs/test/.deps
	@rm -rf spirit/libs/test/Makefile

realclean:: distclean
	@rm -f configure

depend::
	makedepend -Y -fGNUmakefile '-s# Dependencies' `find . -type f -name '*.c*' ` 2>/dev/null
	@sed <GNUmakefile >GNUmakefile.bak -e 's/\.\///g'
	@mv GNUmakefile.bak GNUmakefile

# Dependencies

libscheduler/scheduler.o: libscheduler/scheduler.hh
libscheduler/scheduler.o: libscheduler/pollvector.hh
libscheduler/test.o: libscheduler/scheduler.hh
libscheduler/test.o: libscheduler/pollvector.hh
HTTPParser.o: HTTPParser.hh
config.o: config.hh log.hh
log.o: log.hh
main.o: tcp-listener.hh ScopeGuard/ScopeGuard.hh
main.o: system-error/system-error.hh libscheduler/scheduler.hh
main.o: libscheduler/pollvector.hh log.hh request-handler.hh config.hh
rh-construction.o: system-error/system-error.hh request-handler.hh
rh-construction.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
rh-construction.o: config.hh log.hh
rh-copy-file.o: system-error/system-error.hh request-handler.hh
rh-copy-file.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
rh-copy-file.o: log.hh
rh-errors.o: request-handler.hh libscheduler/scheduler.hh
rh-errors.o: libscheduler/pollvector.hh config.hh log.hh
rh-get-request-body.o: request-handler.hh libscheduler/scheduler.hh
rh-get-request-body.o: libscheduler/pollvector.hh log.hh
rh-get-request-header.o: request-handler.hh libscheduler/scheduler.hh
rh-get-request-header.o: libscheduler/pollvector.hh HTTPParser.hh log.hh
rh-get-request-line.o: request-handler.hh libscheduler/scheduler.hh
rh-get-request-line.o: libscheduler/pollvector.hh HTTPParser.hh
rh-get-request-line.o: urldecode.hh log.hh
rh-log-access.o: system-error/system-error.hh request-handler.hh
rh-log-access.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
rh-log-access.o: config.hh timestamp-to-string.hh log.hh
rh-persistent-connections.o: request-handler.hh libscheduler/scheduler.hh
rh-persistent-connections.o: libscheduler/pollvector.hh config.hh log.hh
rh-readable.o: system-error/system-error.hh request-handler.hh
rh-readable.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
rh-readable.o: config.hh log.hh
rh-setup-reply.o: request-handler.hh libscheduler/scheduler.hh
rh-setup-reply.o: libscheduler/pollvector.hh timestamp-to-string.hh
rh-setup-reply.o: config.hh log.hh
rh-terminate.o: request-handler.hh libscheduler/scheduler.hh
rh-terminate.o: libscheduler/pollvector.hh log.hh
rh-timeouts.o: request-handler.hh libscheduler/scheduler.hh
rh-timeouts.o: libscheduler/pollvector.hh log.hh
rh-writable.o: system-error/system-error.hh request-handler.hh
rh-writable.o: libscheduler/scheduler.hh libscheduler/pollvector.hh
rh-writable.o: log.hh
rh-write-remaining-data.o: request-handler.hh libscheduler/scheduler.hh
rh-write-remaining-data.o: libscheduler/pollvector.hh log.hh
spirit/libs/example/cpp/cpp.o: spirit/libs/example/cpp/cpp.hpp
spirit/libs/example/cpp/cpp.o: spirit/libs/example/cpp/config.hpp
spirit/libs/example/cpp/cpp.o: spirit/libs/example/cpp/cpp_line.hpp
spirit/libs/example/cpp/cpp.o: spirit/libs/example/cpp/cpp_token_definitions.hpp
spirit/libs/example/cpp/cpp.o: spirit/libs/example/cpp/cpp_preprocess.hpp
spirit/libs/example/cpp/cpp.o: spirit/libs/example/cpp/cpp_lexer_slex.hpp
spirit/libs/example/cpp/cpp.o: spirit/libs/example/slex/lexer.hpp
spirit/libs/example/cpp/cpp.o: spirit/libs/example/cpp/time_conversion_helper.hpp
spirit/libs/example/cpp/cpp.o: spirit/libs/example/cpp/cpp_exceptions.hpp
spirit/libs/example/cpp/cpp.o: spirit/libs/example/cpp/cpp_context.hpp
spirit/libs/example/cpp/cpp.o: spirit/libs/example/cpp/path.hpp
spirit/libs/example/cpp/cpp.o: spirit/libs/example/cpp/cpp_macromap.hpp
spirit/libs/example/cpp/cpp.o: spirit/libs/example/cpp/cpp_ifblock.hpp
spirit/libs/example/cpp/cpp.o: spirit/libs/example/cpp/cpp_start_context.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/cpp/cpp.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/cpp/config.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/cpp/version.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/cpp/options.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/cpp/cpp_preprocess.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/cpp/cpp_token_definitions.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/cpp/cpp_line.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/cpp/cpp_lexer_slex.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/slex/lexer.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/cpp/time_conversion_helper.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/cpp/cpp_exceptions.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/cpp/cpp_context.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/cpp/path.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/cpp/cpp_macromap.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/cpp/cpp_ifblock.hpp
spirit/libs/example/cpp/cpp_context.o: spirit/libs/example/cpp/cpp_start_context.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/cpp.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/config.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/cpp_preprocess.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/cpp_token_definitions.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/cpp_line.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/cpp_lexer_slex.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/slex/lexer.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/time_conversion_helper.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/cpp_exceptions.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/cpp_context.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/path.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/cpp_macromap.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/cpp_ifblock.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/cpp_expression_grammar.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/cpp_chlit_grammar.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/cpp_oct_grammar.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/cpp_dec_grammar.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/cpp_hex_grammar.hpp
spirit/libs/example/cpp/cpp_expression_grammar.o: spirit/libs/example/cpp/cpp_double_grammar.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/cpp.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/config.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/parse_tree_utils.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/parse_tree_utils.ipp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/cpp_preprocess.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/cpp_token_definitions.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/cpp_line.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/cpp_lexer_slex.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/slex/lexer.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/time_conversion_helper.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/cpp_exceptions.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/cpp_context.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/path.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/cpp_macromap.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/cpp_ifblock.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/cpp_grammar.hpp
spirit/libs/example/cpp/cpp_grammar.o: spirit/libs/example/cpp/cpp_grammar_rule_ids.hpp
spirit/libs/example/cpp/cpp_lexer_slex.o: spirit/libs/example/cpp/cpp.hpp
spirit/libs/example/cpp/cpp_lexer_slex.o: spirit/libs/example/cpp/config.hpp
spirit/libs/example/cpp/cpp_lexer_slex.o: spirit/libs/example/cpp/cpp_token_definitions.hpp
spirit/libs/example/cpp/cpp_lexer_slex.o: spirit/libs/example/cpp/cpp_lexer_slex.hpp
spirit/libs/example/cpp/cpp_lexer_slex.o: spirit/libs/example/slex/lexer.hpp
spirit/libs/example/cpp/cpp_lexer_slex.o: spirit/libs/example/cpp/time_conversion_helper.hpp
spirit/libs/example/cpp/cpp_lexer_slex.o: spirit/libs/example/cpp/cpp_exceptions.hpp
spirit/libs/example/cpp/cpp_macromap.o: spirit/libs/example/cpp/cpp.hpp
spirit/libs/example/cpp/cpp_macromap.o: spirit/libs/example/cpp/config.hpp
spirit/libs/example/cpp/cpp_macromap.o: spirit/libs/example/cpp/parse_node_iterator.hpp
spirit/libs/example/cpp/cpp_macromap.o: spirit/libs/example/cpp/cpp_preprocess.hpp
spirit/libs/example/cpp/cpp_macromap.o: spirit/libs/example/cpp/cpp_token_definitions.hpp
spirit/libs/example/cpp/cpp_macromap.o: spirit/libs/example/cpp/cpp_line.hpp
spirit/libs/example/cpp/cpp_macromap.o: spirit/libs/example/cpp/cpp_lexer_slex.hpp
spirit/libs/example/cpp/cpp_macromap.o: spirit/libs/example/slex/lexer.hpp
spirit/libs/example/cpp/cpp_macromap.o: spirit/libs/example/cpp/time_conversion_helper.hpp
spirit/libs/example/cpp/cpp_macromap.o: spirit/libs/example/cpp/cpp_exceptions.hpp
spirit/libs/example/cpp/cpp_macromap.o: spirit/libs/example/cpp/cpp_context.hpp
spirit/libs/example/cpp/cpp_macromap.o: spirit/libs/example/cpp/path.hpp
spirit/libs/example/cpp/cpp_macromap.o: spirit/libs/example/cpp/cpp_macromap.hpp
spirit/libs/example/cpp/cpp_macromap.o: spirit/libs/example/cpp/cpp_ifblock.hpp
spirit/libs/example/cpp/cpp_macromap.o: spirit/libs/example/cpp/cpp_defined_grammar.hpp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/cpp/cpp.hpp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/cpp/config.hpp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/cpp/parse_tree_utils.hpp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/cpp/parse_tree_utils.ipp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/cpp/cpp_grammar_rule_ids.hpp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/cpp/cpp_preprocess.hpp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/cpp/cpp_token_definitions.hpp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/cpp/cpp_line.hpp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/cpp/cpp_lexer_slex.hpp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/slex/lexer.hpp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/cpp/time_conversion_helper.hpp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/cpp/cpp_exceptions.hpp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/cpp/cpp_context.hpp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/cpp/path.hpp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/cpp/cpp_macromap.hpp
spirit/libs/example/cpp/cpp_preprocess.o: spirit/libs/example/cpp/cpp_ifblock.hpp
spirit/libs/example/cpp/cpp_start_context.o: spirit/libs/example/cpp/cpp.hpp
spirit/libs/example/cpp/cpp_start_context.o: spirit/libs/example/cpp/config.hpp
spirit/libs/example/cpp/cpp_start_context.o: spirit/libs/example/cpp/cpp_exceptions.hpp
spirit/libs/example/cpp/cpp_start_context.o: spirit/libs/example/cpp/cpp_start_context.hpp
spirit/libs/example/cpp/cpp_start_context.o: spirit/libs/example/cpp/path.hpp
spirit/libs/example/cpp/options.o: spirit/libs/example/cpp/options.hpp
spirit/libs/example/slex/callback.o: spirit/libs/example/slex/lexer.hpp
spirit/libs/example/slex/lexer.o: spirit/libs/example/slex/lexer.hpp
spirit/libs/example/slex/lextest.o: spirit/libs/example/slex/lexer.hpp
spirit/libs/example/xml/ast_xml.o: spirit/libs/example/xml/xml_grammar.hpp
spirit/libs/example/xml/xml.o: spirit/libs/example/xml/xml_grammar.hpp
