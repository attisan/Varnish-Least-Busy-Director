#!/usr/bin/perl -Tw
#-
# Copyright (c) 2006 Linpro AS
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer
#    in this position and unchanged.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# $Id$
#

package Varnish::Test::Case;

use strict;
use base 'Varnish::Test::Object';

sub _init($) {
    my $self = shift;

    &Varnish::Test::Object::_init($self);

    $self->set('assert', \&assert);
}

sub run($) {
    my $self = shift;

    if (!defined($self->{'started'})) {
	print "Start of CASE \"$self->{name}\"...\n";
	$self->{'started'} = 1;
    }

    &Varnish::Test::Object::run($self);

    if ($self->{'finished'}) {
	print "End of CASE \"$self->{name}\".\n";
    }
}

sub assert($$) {
    my $self = shift;
    my $invocation = shift;

    my $bool = $invocation->{'args'}[0]->{'return'};

    if (!$bool) {
	print "  ASSERTION DOES NOT HOLD.\n";
    }
    else {
	print "  Assertion holds.\n";
    }

    $invocation->{'finished'} = 1;
}

1;
