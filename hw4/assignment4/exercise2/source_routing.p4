// TODO: define headers & header instances
header_type pkt_vals {
    fields {
        preamble: 64;
        num_valid: 32;
        port: 8;
        mtype: 8;
        key: 32;
        value: 32;
    }
}

header_type meta_t {
    fields {
        register_tmp : 32;
    }
}

metadata meta_t meta;

header pkt_vals inp;

parser start {
    return select(current(0, 64)) {
        1: extract_pkt;
        default: ingress;
    }
}

// TODO: define parser states
parser extract_pkt {
    extract(inp);
    return ingress;
}

action _drop(){
    drop();
}
action _put(){
    modify_field(standard_metadata.egress_spec, standard_metadata.ingress_port);
    register_write(store, inp.key, inp.value);
    modify_field(inp.mtype, 3);
    add_to_field(inp.num_valid, -1);
}
action _get(){
    modify_field(standard_metadata.egress_spec, standard_metadata.ingress_port);
    register_read(inp.value, store, inp.key);
    modify_field(inp.mtype, 2);
    add_to_field(inp.num_valid, -1);
}

table reg_action{
    reads {
        inp.mtype: exact;
    }
    actions {
        _drop;
        _put;
        _get;
    }
    size: 2;
}
register store{
    width: 32;
    instance_count: 1000;

}
control ingress {
    apply(reg_action);
}

control egress {
    // leave empty
}