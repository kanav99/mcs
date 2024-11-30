use ark_ec::{CurveGroup, VariableBaseMSM};
// We'll use the BLS12-381 G1 curve for this example.
// This group has a prime order `r`, and is associated with a prime field `Fr`.
use ark_test_curves::bls12_381::{G1Projective as G, G1Affine as GAffine, Fr as ScalarField};
use ark_std::{Zero,  UniformRand};
// use std::time::Instant;

const D0: usize = 1000;
const D1: usize = 1000;

#[no_mangle]
extern "C" fn hello_rust() {
    println!("Hello from Rust!");
}

fn mcs_setup() -> Vec<GAffine>
{
    let mut rng = ark_std::test_rng();
    // generate a list of `d1` random group elements
    let mut _setup = Vec::new();
    for _ in 0..D1 {
        _setup.push(GAffine::rand(&mut rng));
    }

    return _setup;
}

// commit a D0 x D1 matrix, m is in column-major order
fn mcs_commit(_setup: &Vec<GAffine>, m: &Vec<Vec<ScalarField>>) -> Vec<GAffine>
{
    let mut _commit = Vec::new();
    for j in 0..D1 {
        let r = G::msm(_setup, &m[j]).unwrap();
        _commit.push(r.into_affine());
    }
    return _commit;
}

fn mcs_eval(m: &Vec<Vec<ScalarField>>, x: &Vec<ScalarField>) -> Vec<ScalarField>
{
    let mut _eval = Vec::new();
    for i in 0..D0 {
        let mut r = ScalarField::zero();
        for j in 0..D1 {
            r += m[j][i] * x[j];
        }
        _eval.push(r);
    }
    
    return _eval;
}

fn mcs_verify(_setup: &Vec<GAffine>, _commit: &Vec<GAffine>, x: &Vec<ScalarField>, y: &Vec<ScalarField>)
{
    let t1 = G::msm(_setup, &y).unwrap();
    let t2 = G::msm(_commit, &x).unwrap();
    assert_eq!(t1, t2);
}

// fn main()
// {
//     let m: Vec<Vec<ScalarField>> = vec![vec![ScalarField::from(1); D0]; D1];
//     let x: Vec<ScalarField> = vec![ScalarField::from(5); D0];

//     let _setup = mcs_setup();
    
//     let _commit = mcs_commit(&_setup, &m);
    
//     let t0 = Instant::now();
//     let y = mcs_eval(&m, &x);
//     let t1 = Instant::now();
//     let duration0 = t1.duration_since(t0);
//     println!("Time elapsed in mcs_eval() is: {:?}", duration0);

//     let t2 = Instant::now();
//     mcs_verify(&_setup, &_commit, &x, &y);
//     let t3 = Instant::now();
//     let duration1 = t3.duration_since(t2);
//     println!("Time elapsed in mcs_verify() is: {:?}", duration1);
// }
