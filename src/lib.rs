use ark_ec::{CurveGroup, VariableBaseMSM};
// We'll use the BLS12-381 G1 curve for this example.
// This group has a prime order `r`, and is associated with a prime field `Fr`.
use ark_test_curves::bls12_381::{G1Projective as G, G1Affine as GAffine, Fr as ScalarField};
use ark_std::{Zero,  UniformRand};
use ark_serialize::{CanonicalSerialize, CanonicalDeserialize};
use bincode;
// use std::time::Instant;

use std::ffi::{c_ulonglong, c_uint};
const MAX_DIM: usize = 1024;

#[no_mangle]
extern "C" fn hello_rust() {
    println!("Hello from Rust!");
}

static mut global_setup: Vec<GAffine> = Vec::new();
static mut global_commit: Vec<GAffine> = Vec::new();

#[no_mangle]
extern "C" fn mcs_gen_setup() // -> Vec<GAffine>
{
    let mut rng = ark_std::test_rng();

    // generate a list of `MAX_DIM` random group elements
    let mut setup = Vec::new();
    for _ in 0..MAX_DIM {
        let pt = GAffine::rand(&mut rng);
        let mut compressed_bytes = Vec::new();
        pt.serialize_compressed(&mut compressed_bytes).unwrap();
        setup.push(compressed_bytes);
    }
    
    // dump the setup to a file
    let setup_bytes = bincode::serialize(&setup).unwrap();
    std::fs::write("setup.bin", &setup_bytes).unwrap();
}

#[no_mangle]
extern "C" fn mcs_load_setup()
{
    let setup_bytes = std::fs::read("setup.bin").unwrap();
    let setup: Vec<Vec<u8>> = bincode::deserialize(&setup_bytes).unwrap();
    unsafe {
        global_setup.clear();
    }
    for i in 0..MAX_DIM {
        let pt = GAffine::deserialize_compressed(&setup[i][..]).unwrap();
        unsafe {
            global_setup.push(pt);
            assert!(global_setup[i].is_on_curve());
        }
    }
}

#[no_mangle]
extern "C" fn mcs_gen_commit(_d0: c_uint, _d1: c_uint, _m: *const c_ulonglong)
{
    let d0 = _d0 as usize;
    let d1 = _d1 as usize;
    let m = unsafe { std::slice::from_raw_parts(_m, d0 * d1) };
    assert!(d1 < MAX_DIM);
    
    // convert m to a matrix in column-major order
    let mut mat = Vec::new();
    for j in 0..d1 {
        let mut col = Vec::new();
        for i in 0..d0 {
            col.push(ScalarField::from(m[i * d1 + j]));
        }
        mat.push(col);
    }
    

    let mut _commit = Vec::new();
    for j in 0..d1 {
        unsafe {
            let r = G::msm(&global_setup[..d0], &mat[j]).unwrap().into_affine();
            let mut compressed_bytes = Vec::new();
            r.serialize_compressed(&mut compressed_bytes).unwrap();
            _commit.push(compressed_bytes);
        }
    }

    // dump the commit to a file
    let commit_bytes = bincode::serialize(&_commit).unwrap();
    std::fs::write("commit.bin", &commit_bytes).unwrap();
}

#[no_mangle]
extern "C" fn mcs_load_commit()
{
    let commit_bytes = std::fs::read("commit.bin").unwrap();
    let commit: Vec<Vec<u8>> = bincode::deserialize(&commit_bytes).unwrap();
    unsafe {
        global_commit.clear();
    }
    for i in 0..commit.len() {
        let r = GAffine::deserialize_compressed(&commit[i][..]).unwrap();
        unsafe {
            global_commit.push(r);
            assert!(global_commit[i].is_on_curve());
        }
    }
}

#[no_mangle]
extern "C" fn mcs_verify(_d0: c_uint, _d1: c_uint, _x: *const c_ulonglong, _y: *const c_ulonglong) -> c_uint
{
    let d0 = _d0 as usize;
    let d1 = _d1 as usize;
    let x = unsafe { std::slice::from_raw_parts(_x, d1) };
    let y = unsafe { std::slice::from_raw_parts(_y, d0) };
    let x_field = x.iter().map(|&x| ScalarField::from(x)).collect::<Vec<_>>();
    let y_field = y.iter().map(|&y| ScalarField::from(y)).collect::<Vec<_>>();
    assert!(d1 < MAX_DIM);
    unsafe {
        assert!(global_commit.len() == d1);
        let t1 = G::msm(&global_setup[..d0], &y_field).unwrap();
        let t2 = G::msm(&global_commit[..d1], &x_field).unwrap();
        if t1 == t2 {
            return 1;
        } else {
            return 0;
        }
    }
}
