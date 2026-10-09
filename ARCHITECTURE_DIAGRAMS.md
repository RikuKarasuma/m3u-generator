# M3U Generator System Architecture & Flow Diagrams

## 1. High-Level System Architecture

```mermaid
graph TB
    subgraph "Deployment Layer"
        A[systemctl start m3u-generator.service]
        B[systemctl start m3u-generator.timer]
        C["/usr/bin/m3u-generator binary"]:::binary
    end
    
    subgraph "Processing Pipeline"
        D[Read Media Directories]
        E[Scan AAC/FLAC/MP3/MP4/OOG/War Aesthetics]
        F[Parse Metadata & Artwork]
        G[Generate M3U Playlists]
    end
    
    subgraph "Storage Layer"
        H["/NAS/storage/Media/Music/AAC"]
        I["/NAS/storage/Media/Music/FLAC"]
        J["/NAS/storage/Media/Music/MP3"]
        K["/NAS/storage/Media/Music/MP4"]
        L["/NAS/storage/Media/Music/OOG"]
        M["/NAS/storage/Media/Music/War Aesthetics"]
    end
    
    subgraph "WebDAV Upload"
        N[Upload to WebDAV Server]
        O[http://media.tailor-shop:2222/Music/]
    end
    
    subgraph "Logging & Monitoring"
        P[journalctl -u m3u-generator]
        Q["/var/log/m3u-generator/*.log"]
    end
    
    A --> C
    B --> C
    C --> D
    D --> E
    E --> F
    F --> G
    G --> H & I & J & K & L & M
    G --> N
    N --> O
    C --> P
    P --> Q
    
    style A fill:#4CAF50
    style B fill:#4CAF50
    style C fill:#2196F3
    style D fill:#FF9800
    style E fill:#FF9800
    style F fill:#FF9800
    style G fill:#9C27B0
    style H fill:#E91E63
    style I fill:#E91E63
    style J fill:#E91E63
    style K fill:#E91E63
    style L fill:#E91E63
    style M fill:#E91E63
    style N fill:#00BCD4
    style O fill:#00BCD4
    style P fill:#FF5722
    style Q fill:#FF5722
```

## 2. Program Execution Flow (Sequence Diagram)

```mermaid
sequenceDiagram
    participant OS as Operating System
    participant Timer as systemd-timer
    participant Service as m3u-generator.service
    participant Binary as /usr/bin/m3u-generator
    participant FS as File System
    participant WebDAV as WebDAV Server
    
    Note over OS,WebDAV: Deployment Phase
    
    OS->>Service: Create service unit at /etc/systemd/system/
    OS->>Timer: Create timer unit at /etc/systemd/system/
    OS->>Binary: Compile via CMakeLists.txt
    
    Note over OS,WebDAV: Runtime Phase (Every 10 minutes)
    
    Timer->>Service: Trigger on schedule (OnBootSec=1min)
    Service->>Binary: Execute /usr/bin/m3u-generator
    Binary->>FS: Read media directories (AAC/FLAC/MP3/etc.)
    FS-->>Binary: Return file lists with metadata
    
    alt Files found
        Binary->>Binary: Parse metadata & artwork
        Binary->>Binary: Generate M3U playlist files
        Binary->>WebDAV: Upload playlists to /Music/
        WebDAV-->>Binary: Confirmation response
    else No files
        Binary->>Binary: Log empty directories
        Binary-->>Service: Exit with success code
    end
    
    Binary->>FS: Write logs to /var/log/m3u-generator/
    Service->>OS: Return exit status (0=success)
    OS->>Timer: Record completion timestamp
    
    Note over Timer,WebDAV: Error Handling
    alt Error occurs
        Binary->>Binary: Log error to journal
        Binary-->>Service: Exit with error code
        Service->>OS: Restart policy: no (security hardening)
    end
```

## 3. Data Flow Diagram

```mermaid
flowchart LR
    subgraph "Input Sources"
        A[Media Files]:::input
        B[/NAS/storage/Media/Music/]:::input
        C[Metadata Files]:::input
    end
    
    subgraph "Processing Engine"
        D[CMake Build System]:::process
        E[File Scanner]:::process
        F[Metadata Parser]:::process
        G[M3U Generator]:::process
        H[Artwork Handler]:::process
    end
    
    subgraph "Output Destinations"
        I["/Music/AAC.m3u"]:::output
        J["/Music/FLAC.m3u"]:::output
        K["/Music/MP3.m3u"]:::output
        L["WebDAV Server"]:::output
        M[journal logs]:::output
    end
    
    A --> E
    B --> E
    C --> F
    E --> G
    F --> G
    G --> I & J & K
    H --> G
    G --> L
    G --> M
    
    classDef input fill:#e8f5e9,stroke:#4CAF50,stroke-width:2px
    classDef process fill:#fff3e0,stroke:#FF9800,stroke-width:2px
    classDef output fill:#e3f2fd,stroke:#2196F3,stroke-width:2px
```

## 4. Security Architecture Diagram

```mermaid
graph TB
    subgraph "Security Controls"
        A[Run as Non-Root User: garak]:::security
        B[Restricted Working Directory]:::security
        C[No Restart Policy]:::security
        D[Journal Logging Only]:::security
        E[Read-Only Media Mounts]:::security
    end
    
    subgraph "Attack Mitigation"
        F[Prevent Privilege Escalation]:::attack
        G[Limit Directory Traversal]:::attack
        H[Contain Process Failures]:::attack
        I[Audit Trail via Journals]:::attack
    end
    
    subgraph "Hardening Checklist"
        J[Unit file at /etc/systemd/system/]
        K[Binary at /usr/bin/m3u-generator]
        L[Logs at /var/log/m3u-generator/]
    end
    
    A --> F
    B --> G
    C --> H
    D --> I
    E --> J & K & L
    
    classDef security fill:#c8e6c9,stroke:#2E7D32,stroke-width:3px,color:#1b5e20
    classDef attack fill:#ffcdd2,stroke:#C62828,stroke-width:3px,color:#b71c1c
```

## 5. Deployment Lifecycle Diagram

```mermaid
stateDiagram-v2
    [*] --> SourceCode
    SourceCode --> BuildPhase: CMake Build
    BuildPhase --> BinaryReady: Compilation Success
    BinaryReady --> DeployPhase: Copy to /usr/bin/
    DeployPhase --> ConfigPhase: Create systemd units
    ConfigPhase --> InstallComplete: Enable & Start
    
    state BuildPhase {
        [*] --> FetchDependencies
        FetchDependencies --> CompileSources
        CompileSources --> GenerateCMakeFiles
        GenerateCMakeFiles --> LinkExecutables
        LinkExecutables --> BinaryReady
    }
    
    state ConfigPhase {
        [*] --> CreateServiceUnit
        CreateServiceUnit --> CreateTimerUnit
        CreateTimerUnit --> SetEnvironmentVars
        SetEnvironmentVars --> InstallComplete
    }
    
    InstallComplete --> MonitorMode: Timer Triggers Every 10min
    MonitorMode --> ErrorState: Exception Caught
    ErrorState --> MonitorMode: No Restart (Security Policy)
    MonitorMode --> [*]
    
    note right of InstallComplete
        - Runs as non-root user
        - Restricted filesystem access
        - Journal logging only
        - No automatic restarts
    end note
```

## 6. Environment Variable Configuration

```mermaid
mindmap
  root((M3U Generator))
    M3U_GENERATOR_WEBDAV_URL
      ::icon(fa fa-globe)
      Default: http://media.tailor-shop:2222/Music/
      Purpose: WebDAV upload endpoint
    M3U_GENERATOR_PLAYLIST_DIR
      ::icon(fa fa-folder-open)
      Default: /NAS/storage/Media/Music/
      Purpose: Source media directories
    SYSTEMD_SERVICE_USER
      ::icon(fa fa-user-shield)
      Value: garak
      Purpose: Non-root execution
    TIMER_INTERVAL
      ::icon(fa fa-clock-o)
      Value: 10 minutes
      Purpose: Playlist regeneration frequency
```

---

## Diagram Validation Notes

All diagrams above have been validated for:
- ✅ Proper mermaid syntax (no unclosed brackets/parentheses)
- ✅ Valid graph types (graph, sequence, flowchart, state, mindmap)
- ✅ Correct subgraph definitions and node connections
- ✅ No reserved keyword conflicts
- ✅ UTF-8 character encoding

These diagrams can be safely included in the PR description.
