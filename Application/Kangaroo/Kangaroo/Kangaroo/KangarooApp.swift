//
//  KangarooApp.swift
//  Kangaroo
//
//  Created by MKonzerowsky on 7/29/24.
//

import SwiftUI

@main
struct KangarooApp: App {
    let persistenceController = PersistenceController.shared

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environment(\.managedObjectContext, persistenceController.container.viewContext)
        }
    }
}
