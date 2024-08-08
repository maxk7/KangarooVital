//
//  ContentView.swift
//  Kangaroo
//
//  Created by MKonzerowsky on 7/29/24.
//

import SwiftUI
import SlideOverCard
import FirebaseDatabase

struct ContentView: View {
    @State private var name: String = ""
    @State private var appValue: Int = 0
    @State private var ardValue: Int = 0
    @State private var vibrationTimer: Timer?
    @State private var isImageLoaded: Bool = false
    @State private var isPresented = false
    
    @State private var imageScale: CGFloat = 1.0 // State property to track the image scale
    @State private var shadowRadius: CGFloat = 3.0 // State property to track the shadow radius
    @State private var circlePulses: [UUID] = [] // State property to track circle pulses

    let firstPulseGenerator = UIImpactFeedbackGenerator(style: .medium) // Weaker overall strength
    let secondPulseGenerator = UIImpactFeedbackGenerator(style: .light) // Weaker second pulse

    let colorLightPink = Color(red: 1, green: 0.89, blue: 0.90)
    let colorVibrantPink = Color(red: 1, green: 0.447, blue: 0.541)
    let colorDarkPink = Color(red: 1, green: 0.447, blue: 0.541)
    
    var body: some View {
        ZStack {
            if isImageLoaded {
                mainContent
            } else {
                colorLightPink.edgesIgnoringSafeArea(.all)
                
                ProgressView("Loading...") // Loading indicator
                    .progressViewStyle(CircularProgressViewStyle())
            }
        }
        .onAppear {
            loadImage()
            getArdValue()
            getAppValue()
            getNameValue()
        }
    }
    
    var mainContent: some View {
        ZStack {
            colorLightPink.edgesIgnoringSafeArea(.all)
            
            VStack {
                Spacer()
                
                ZStack {
                    ForEach(circlePulses, id: \.self) { pulse in
                        CirclePulseView()
                    }
                    
                    Image(uiImage: UIImage(named: "HeartIcon")!)
                        .resizable()
                        .scaledToFit()
                        .frame(width: 300, height: 300)
                        .scaleEffect(imageScale)
                }
                
                Text("\(appValue) bpm")
                    .font(.custom("Rubik", size: 50))
                    .background(.clear)
                    .foregroundColor(colorVibrantPink)
                    .shadow(color: colorVibrantPink.opacity(0.5), radius: shadowRadius)
            
                Spacer()
                
                Image(systemName: "chevron.compact.up")
                    .resizable()
                    .scaledToFit()
                    .frame(width: 28, height: 28)
                    .onTapGesture { self.isPresented = true }
                    .foregroundColor(colorDarkPink)
                    .padding(.bottom, -5)
                
                Text("Your Heart Rate is Synced")
                .font(.custom("Rubik", size: 18))
                .fontWeight(.bold)
                .foregroundColor(colorDarkPink)
                .padding(.bottom, 10)
            }
        }
        .slideOverCard(isPresented: $isPresented, style: SOCStyle(continuous: false, innerPadding: 16.0, outerPadding: 20.0, style: colorLightPink)) {
            Stepper(value: $ardValue, in: 55...85, step: 1) {
                Label("Your HR: \(ardValue) bpm", systemImage: "suit.heart.fill")
            }
            .onChange(of: ardValue) { setArdValue() }
        }
    }

    func loadImage() {
        // Load the image from assets
        if let _ = UIImage(named: "HeartIcon") {
            // Image is successfully loaded
            self.isImageLoaded = true
        } else {
            // Handle the case where the image failed to load
            // This example assumes the image will always be there, so it sets isImageLoaded to true for simplicity
            self.isImageLoaded = false
        }
    }

    func setArdValue() {
        let ref = Database.database().reference()
        ref.child("users/example_user_id/ard").setValue(ardValue)
    }
    
    func getNameValue() {
        let ref = Database.database().reference()
        ref.child("users/example_user_id/name").observe(.value) { snapshot in
            if let value = snapshot.value as? String {
                name = value
            }
        }
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
    
    func getArdValue() {
        let ref = Database.database().reference()
        ref.child("users/example_user_id/ard").observe(.value) { snapshot in
            if let value = snapshot.value as? Int {
                ardValue = value
            }
        }
    }
    
    func triggerShadowAnimation(duration: Double) {
        withAnimation(.easeInOut(duration: duration * 0.3)) {
            self.shadowRadius = 6
        }
        DispatchQueue.main.asyncAfter(deadline: .now() + duration * 0.3) {
            withAnimation(.easeInOut(duration: duration * 0.7)) {
                self.shadowRadius = 3
            }
        }
    }
    
    func startVibration() {
        // Cancel the existing timer if it exists
        vibrationTimer?.invalidate()
        
        let bpm = appValue
        let interval = 60.0 / Double(bpm)
        let firstPulseDuration: TimeInterval = 0.04
        let secondPulseStart: TimeInterval = 0.140
        let secondPulseDuration: TimeInterval = 0.04
        let animationFactor: Double = 2.2
        
        firstPulseGenerator.prepare()
        secondPulseGenerator.prepare()
        
        vibrationTimer = Timer.scheduledTimer(withTimeInterval: interval, repeats: true) { timer in
            self.firstPulseGenerator.impactOccurred()
            
            triggerShadowAnimation(duration: interval)
            
            // First pulse
            DispatchQueue.main.asyncAfter(deadline: .now() + firstPulseDuration) {
                self.firstPulseGenerator.prepare()
                
                withAnimation(.easeInOut(duration: firstPulseDuration * animationFactor)) {
                    self.imageScale = 1.1
                    self.circlePulses.append(UUID())
                }
            }
            
            // Second pulse
            DispatchQueue.main.asyncAfter(deadline: .now() + secondPulseStart) {
                self.secondPulseGenerator.impactOccurred()
                
                withAnimation(.easeInOut(duration: secondPulseDuration * animationFactor)) {
                    self.imageScale = 1.02
                    self.circlePulses.append(UUID())
                }
                
                DispatchQueue.main.asyncAfter(deadline: .now() + secondPulseDuration) {
                    self.secondPulseGenerator.prepare()
                    withAnimation(.easeInOut(duration: secondPulseDuration * animationFactor)) {
                        self.imageScale = 1.0
                    }
                }
            }
        }
    }
}

struct CirclePulseView: View {
    @State private var animate = false
    
    let colorVibrantPink = Color(red: 1, green: 0.447, blue: 0.541)
    
    var body: some View {
        Circle()
            .fill(colorVibrantPink.opacity(0.1))
            .frame(width: 220, height: 220)
            .scaleEffect(animate ? 2.5 : 0)
            .opacity(animate ? 0 : 1)
            .animation(Animation.easeOut(duration: 4.0), value: animate)

            .onAppear {
                animate = true
            }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
