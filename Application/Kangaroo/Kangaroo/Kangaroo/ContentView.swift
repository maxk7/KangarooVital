//
//  ContentView.swift
//  Kangaroo
//
//  Created by MKonzerowsky on 7/29/24.
//

import SwiftUI
import FirebaseDatabase

struct ContentView: View {
    @State private var appValue: Int = 0
    @State private var ardValue: Int = 0

    var body: some View {
        VStack {
            Text("App Value: \(appValue)")
            Text("ARD Value: \(ardValue)")
            Button(action: setAppValue) {
                Text("Set App Value to 42")
            }
            Button(action: getArdValue) {
                Text("Get ARD Value")
            }
        }
        .padding()
        .onAppear(perform: getArdValue)
    }

    func setAppValue() {
        let ref = Database.database().reference()
        ref.child("users/example_user_id/app").setValue(42)
    }

    func getArdValue() {
        let ref = Database.database().reference()
        ref.child("users/example_user_id/ard").observeSingleEvent(of: .value) { snapshot in
            if let value = snapshot.value as? Int {
                ardValue = value
            }
        }
    }
}
