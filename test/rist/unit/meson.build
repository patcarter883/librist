cmocka = meson.get_compiler('c').find_library('cmocka', required: false)

if cmocka.found()
	if have_srp
		srp_unit = executable('srp_unit', rev_target,
								'srp_examples.c',
								'../../../contrib/pthread-shim.c',
								'../../../src/crypto/random.c',
								include_directories : inc,
								dependencies : [threads, cmocka,crypto_deps],
		)

		test('srp_unit_test', srp_unit, suite:['unit'])
	endif
endif

# Regression test for #208: rist_logging_set with a remote (udp://)
# address must succeed before any rist_*_create call. Does not require
# cmocka - links against the public librist API only.
logging_init_order = executable('logging_init_order',
                                'logging_init_order.c',
                                include_directories : inc,
                                link_with : librist)
test('logging_init_order', logging_init_order, suite:['unit'])