# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Omi is an open-source AI wearable platform consisting of:
- **Hardware devices**: Physical wearable devices (Omi and OmiGlass) with firmware
- **Mobile app**: Flutter-based companion app for iOS/Android
- **Backend**: FastAPI Python server with real-time transcription and AI processing
- **Web apps**: Next.js frontends for documentation and AI personas
- **SDKs**: React Native, Python, Swift, and Expo SDKs
- **MCP server**: Model Context Protocol server for AI integration

## Development Commands

### Backend (FastAPI + Python)

**Prerequisites:**
- Python 3.x
- Redis (Upstash recommended)
- ngrok for local testing
- Required API keys: OpenAI, Deepgram, Pinecone

**Commands:**
```bash
cd backend

# Install dependencies (use virtual environment)
python -m venv venv
source venv/bin/activate  # On macOS/Linux
# or: venv\Scripts\activate  # On Windows
pip install -r requirements.txt

# Run development server
uvicorn main:app --reload --env-file .env

# Deactivate virtual environment when done
deactivate
```

**Environment setup:**
- Copy `.env.template` to `.env` and fill in required API keys
- Set up ngrok tunnel: `ngrok http --domain=your-domain.ngrok-free.app 8000`

### Mobile App (Flutter)

**Prerequisites:**
- Flutter SDK (v3.35.3+)
- Opus Codec
- For iOS: Xcode (v16.4+), CocoaPods (v1.16.2+)
- For Android: Android Studio, Android SDK Platform API 35, JDK 21, Gradle 8.10, NDK 28.2.13676358

**Commands:**
```bash
cd app

# Setup (one-time)
bash setup.sh ios      # For iOS
bash setup.sh android  # For Android
bash setup.sh macos    # For macOS

# Run development build
flutter run --flavor dev

# Build for release
flutter build ios --flavor dev --release
flutter build android --flavor dev --release

# Install to iPhone
ios-deploy --bundle build/ios/iphoneos/Runner.app --debug
```

### Web Frontend (Next.js)

**Commands:**
```bash
cd web/frontend
npm install
npm run dev        # Development server
npm run build      # Production build
npm run lint       # Run ESLint
npm run lint:fix   # Fix linting issues
```

**For AI Personas:**
```bash
cd web/personas-open-source
npm install
npm run dev
npm run build
```

### OmiGlass

**Commands:**
```bash
cd omiGlass
npm install        # or yarn install
npm start          # or yarn start
```

**Firmware:**
- Use Arduino IDE or arduino-cli to build and upload firmware
- Configure PSRAM setting to "OPI PSRAM" in Arduino IDE

### MCP Server

**Development:**
```bash
cd mcp
# Run tests
python -m pytest tests/
```

## Architecture

### Backend Architecture

The backend is a FastAPI application deployed on Modal with the following structure:

**Core Components:**
- `main.py`: Application entry point, router registration, Modal deployment config
- `routers/`: API endpoint handlers organized by domain (25+ routers)
  - `transcribe.py`: WebSocket-based real-time audio transcription (core functionality)
  - `conversations.py`: Conversation/memory CRUD operations
  - `apps.py`: Plugin/app integrations
  - `auth.py`: Core authentication for main app
  - `oauth.py`: OAuth for Omi apps
  - `chat.py`, `memories.py`, `action_items.py`, etc.
- `database/`: Database access layer (Firestore + Redis)
  - Firestore for persistent storage (users, conversations, apps)
  - Redis for caching and real-time state
- `utils/`: Shared utilities
  - `stt/`: Speech-to-text services (Deepgram, Soniox, Speechmatics)
  - `llm/`: LLM integrations (OpenAI, etc.)
  - `conversations/`: Conversation processing logic
  - `app_integrations.py`: External webhook/integration triggers
- `models/`: Pydantic data models

**Real-time Transcription Flow:**
1. WebSocket connection established via `/v4/listen`
2. Audio streamed from device → STT service (Deepgram/Soniox/Speechmatics)
3. Transcript segments stored in Firestore as "in-progress" conversations
4. Conversation finalized after timeout (configurable, default 120s)
5. AI processing: structured extraction, action items, integrations triggered
6. Results pushed to client via WebSocket and stored in Firestore

**Key Design Patterns:**
- WebSocket for bidirectional real-time communication
- Background tasks for periodic operations (usage recording, conversation lifecycle)
- Pusher service for forwarding audio/transcripts to external services
- Speaker identification using speech profiles and text analysis
- Translation service for multi-language support
- Photo processing for OmiGlass image capture and description

### Mobile App Architecture

Flutter app organized by feature:
- `lib/pages/`: Screen components
- `lib/providers/`: State management (Provider pattern)
- `lib/backend/`: API client for backend communication
- `lib/services/`: Platform services (BLE, audio, notifications)
- `lib/models/`: Data models matching backend schemas
- `lib/widgets/`: Reusable UI components
- `lib/core/`: App initialization and shell
- `lib/mobile/` and `lib/desktop/`: Platform-specific code

**Key Features:**
- Bluetooth connection to Omi hardware device
- Real-time audio streaming to backend
- Background service for continuous recording
- Offline support with local storage
- Firebase authentication
- Push notifications

### Plugin System

Apps/plugins can:
- Receive conversation webhooks
- Access real-time transcript events
- Execute custom OAuth flows
- Use structured conversation data (title, summary, action items, category)

Location: `plugins/` for examples, backend manages app registry in Firestore

## Testing

**Backend:**
- No centralized test suite found
- Individual plugin tests exist (e.g., `plugins/composio/test_setup.py`)

**MCP:**
- Tests in `mcp/tests/test_server.py`
- Run: `python -m pytest mcp/tests/`

**Mobile App:**
- Integration tests in `app/integration_test/`
- Run: `flutter test`

## Common Workflows

### Adding a New Backend Endpoint

1. Create router file in `backend/routers/` or add to existing router
2. Define Pydantic models in `backend/models/` if needed
3. Add database functions in `backend/database/` if needed
4. Register router in `backend/main.py`
5. Add authentication via `Depends(auth.get_current_user_uid)` if needed

### Working with Conversations/Memories

Conversations are the core data structure:
- Created during real-time transcription sessions
- Stored with status: `in_progress`, `processing`, or `completed`
- Contain: transcript segments, photos, geolocation, structured data
- Processed asynchronously after finalization
- Can trigger external integrations and apps

Access via:
- Backend: `database/conversations.py` functions
- API: `/v1/conversations/*` endpoints
- MCP: `get_conversations`, `create_memory`, etc. tools

### Modifying STT Pipeline

STT services configured in `utils/stt/streaming.py`:
- Service selection based on language
- Speech profile support for speaker identification
- Multi-language detection support

Supported services: Deepgram, Soniox, Speechmatics

### Working with Apps/Plugins

Apps are third-party integrations:
- Registered in Firestore via `/v1/apps` endpoints
- Can specify capabilities: webhooks, OAuth, etc.
- Triggered after conversation processing
- Examples in `plugins/` directory

## Deployment

**Backend:**
- Deployed on Modal (serverless platform)
- Configuration in `backend/modal/Dockerfile`
- Uses Modal secrets for environment variables

**Mobile App:**
- iOS: Xcode build and App Store Connect
- Android: Android Studio build and Google Play Console
- Continuous deployment via app release workflows

**Web:**
- Next.js apps deployed to Vercel or similar
- Dockerfiles available for self-hosting

## Important Notes

- **Firebase is required**: Both app and backend use Firebase (Auth, Firestore, Messaging)
- **WebSocket timeout**: Default inactivity timeout is 30 seconds, conversation timeout is configurable
- **Audio formats**: Backend supports Opus and PCM codecs at various sample rates
- **Language support**: Multi-language transcription with automatic detection and translation
- **Rate limiting**: Transcription credits enforced based on user subscription plan
- **Private cloud sync**: Optional feature for syncing audio to user's storage

## Resources

- Main docs: https://docs.omi.me/
- Discord: http://discord.omi.me
- Hardware buying guide: https://docs.omi.me/doc/assembly/Buying_Guide/
- Plugin development: https://docs.omi.me/doc/developer/apps/Introduction
