//
//  ContentView.swift
//  Kangaroo
//
//  Created by MKonzerowsky on 7/29/24.
//

import SwiftUI
import FirebaseDatabase

struct ContentView: View {
    @State private var appValue: Int = 85
    @State private var ardValue: Int = 65
    
    let notificationGenerator = UINotificationFeedbackGenerator()

    var body: some View {
        ZStack {
            Color(red: 1, green: 0.89, blue: 0.90).edgesIgnoringSafeArea(.all)
            
            VStack {
                Text("App Value: \(appValue)")
                Text("The BPM of this device")
                    .padding(EdgeInsets(top: 0, leading: 0,
                                        bottom: 150, trailing: 0))
                
                Stepper(value: $ardValue, in: 55...85, step: 1) {
                    Text("Set ARD Value: \(ardValue)")
                }
                .onChange(of: ardValue) {setArdValue()}
                .padding()
            }
            .padding()
        }
        .onAppear {
            getAppValue()
        }
    }

    func setArdValue() {
        let ref = Database.database().reference()
        ref.child("users/example_user_id/ard").setValue(ardValue)
    }
    
    func getAppValue() {
        let ref = Database.database().reference()
        ref.child("users/example_user_id/app").observe(.value) { snapshot in
            if let value = snapshot.value as? Int {
                appValue = value
                startVibration()
            }
        }
    }
    
    func startVibration() {
        let bpm = appValue
        let interval = 60.0 / Double(bpm)
        
        Timer.scheduledTimer(withTimeInterval: interval, repeats: true) { timer in
            self.notificationGenerator.notificationOccurred(.success)
            
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.1) {
                self.notificationGenerator.prepare()
            }
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
