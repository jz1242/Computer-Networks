/*
Copyright 2013-present Barefoot Networks, Inc. 

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// TODO: define headers & header instances
header_type easy_route {
    fields {
        preamble: 64;
        num_valid: 32;
    }
}

header_type easy_port {
    fields {
        port: 8;
    }
}

header easy_route easy_route_header;

header easy_port easy_route_port;

parser start {
    return select(current(0, 64)) {
        0: easy_route_header_parse;
        default: ingress;
    }
}

// TODO: define parser states
parser easy_route_header_parse {
    extract(easy_route_header);
    return select(latest.num_valid) {
        0: ingress;
        default: easy_route_port_parse;
    }
}

parser easy_route_port_parse {
    extract(easy_route_port);
    return ingress;
}

action _drop() {
    drop();
}
action route_packet() {
    modify_field(standard_metadata.egress_spec, easy_route_port.port);
    subtract_from_field(easy_route_header.num_valid, 1);
    remove_header(easy_route_port);
}

table routing {
    reads {
        easy_route_port: valid;
    }
    actions {
        _drop;
        route_packet;
    }
    size: 1;
}

control ingress {
    apply(routing);
}

control egress {
    // leave empty
}
