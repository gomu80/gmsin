# CLAUDE.md - AI Assistant Guide for gmsin

This file provides comprehensive guidance for AI assistants working with the gmsin repository. It documents the codebase structure, development workflows, and key conventions.

## Table of Contents

- [Repository Overview](#repository-overview)
- [Codebase Structure](#codebase-structure)
- [Development Workflow](#development-workflow)
- [Key Conventions](#key-conventions)
- [Working with This Repository](#working-with-this-repository)
- [Testing Strategy](#testing-strategy)
- [Common Tasks](#common-tasks)

---

## Repository Overview

### Project Name
**gmsin** - [Add project description here]

### Current Status
🆕 **New Repository** - This is a fresh repository ready for initial development.

### Technology Stack
*To be determined as the project develops. Update this section with:*
- Programming language(s)
- Framework(s)
- Database(s)
- Build tools
- Testing frameworks
- Deployment platforms

### Purpose
[Document the main purpose and goals of this project]

---

## Codebase Structure

*As the project grows, document the directory structure here. Example:*

```
gmsin/
├── src/              # Source code
│   ├── components/   # Reusable components
│   ├── services/     # Business logic services
│   ├── utils/        # Utility functions
│   └── config/       # Configuration files
├── tests/            # Test files
├── docs/             # Additional documentation
├── scripts/          # Build and deployment scripts
└── CLAUDE.md         # This file
```

### Key Directories

**As directories are created, document them here with their purpose:**

- `/src` - Main source code directory
- `/tests` - All test files (unit, integration, e2e)
- `/docs` - Project documentation
- `/scripts` - Automation and helper scripts
- `/config` - Configuration files for different environments

---

## Development Workflow

### Branch Strategy

- **Main Branch**: `main` (or document the actual main branch name)
- **Feature Branches**: Use format `claude/<descriptive-name>-<session-id>`
- **Branch Naming**: Use kebab-case for readability

### Git Workflow

1. **Starting Work**
   ```bash
   git checkout -b claude/feature-name-session-id
   ```

2. **Making Changes**
   - Make focused, logical commits
   - Write clear commit messages following conventional commits format
   - Example: `feat: add user authentication`, `fix: resolve login bug`

3. **Committing**
   ```bash
   git add <files>
   git commit -m "type: description"
   ```

4. **Pushing Changes**
   ```bash
   git push -u origin <branch-name>
   ```
   - Always use the full branch name starting with `claude/`
   - Push will fail with 403 if branch doesn't match the expected pattern

### Commit Message Format

Follow conventional commits:
- `feat:` - New feature
- `fix:` - Bug fix
- `docs:` - Documentation changes
- `refactor:` - Code refactoring
- `test:` - Adding or updating tests
- `chore:` - Maintenance tasks
- `style:` - Code style changes (formatting, etc.)

---

## Key Conventions

### Code Style

*Update this section as coding standards are established:*

- **Indentation**: [spaces/tabs, size]
- **Line Length**: [max characters]
- **Naming Conventions**:
  - Variables: [camelCase/snake_case]
  - Functions: [camelCase/snake_case]
  - Classes: [PascalCase]
  - Constants: [UPPER_CASE]
  - Files: [kebab-case/camelCase]

### File Organization

- Keep files focused on a single responsibility
- Use clear, descriptive file names
- Group related files in directories
- Separate concerns (business logic, UI, data access)

### Documentation

- Document complex algorithms and business logic
- Use inline comments sparingly - prefer self-documenting code
- Keep README files up to date
- Document public APIs and interfaces
- Update this CLAUDE.md as the project evolves

### Error Handling

*Document error handling patterns:*
- How to handle exceptions
- Error logging conventions
- User-facing error messages

### Security Best Practices

- Never commit secrets, API keys, or credentials
- Use environment variables for sensitive configuration
- Validate all user inputs
- Sanitize data to prevent injection attacks
- Follow OWASP Top 10 guidelines
- Keep dependencies updated

---

## Working with This Repository

### For AI Assistants

When working on this repository, follow these guidelines:

#### 1. Understanding the Codebase

- **First Time**: Read through this CLAUDE.md completely
- **Before Changes**: Use Task/Explore tools to understand the current code structure
- **Pattern Matching**: Look for existing patterns and follow them
- **When Uncertain**: Search for similar implementations in the codebase

#### 2. Making Changes

- **Plan First**: Use TodoWrite to break down complex tasks
- **Read Before Edit**: Always read files before making changes
- **Maintain Consistency**: Follow existing code style and patterns
- **Test Changes**: Verify changes work as expected
- **Security Check**: Review code for security vulnerabilities

#### 3. Tool Usage Preferences

- Use **Read** for reading files (not `cat`)
- Use **Edit** for modifying files (not `sed`)
- Use **Write** only for new files
- Use **Grep** for searching code (not `grep` command)
- Use **Glob** for finding files (not `find` command)
- Use **Task/Explore** for broad codebase exploration
- Use **Bash** only for git, package managers, and system commands

#### 4. Communication

- Provide clear, concise updates
- Reference code locations using `file:line` format
- Explain the "why" behind decisions
- Be honest about limitations or uncertainties

#### 5. Quality Standards

- Write clean, maintainable code
- Ensure changes don't break existing functionality
- Add tests for new features
- Update documentation as needed
- Check for security vulnerabilities (XSS, SQL injection, etc.)

---

## Testing Strategy

*Document testing approach as it develops:*

### Test Structure
[Describe how tests are organized]

### Running Tests
```bash
# Command to run tests
```

### Test Coverage
- Target coverage: [percentage]
- Critical paths must have tests
- Test edge cases and error conditions

### Types of Tests
- **Unit Tests**: Test individual functions/components
- **Integration Tests**: Test component interactions
- **E2E Tests**: Test full user workflows
- **Performance Tests**: Test performance requirements

---

## Common Tasks

### Initial Setup

```bash
# Clone the repository
git clone <repository-url>
cd gmsin

# Install dependencies (update when applicable)
# npm install / pip install -r requirements.txt / etc.

# Set up environment
# Copy .env.example to .env and configure
```

### Development

```bash
# Start development server (update when applicable)
# npm run dev / python main.py / etc.

# Run tests
# npm test / pytest / etc.

# Build for production
# npm run build / make build / etc.
```

### Adding a New Feature

1. Create a feature branch
2. Plan the implementation (use TodoWrite)
3. Implement the feature following existing patterns
4. Add tests for the new feature
5. Update documentation
6. Commit and push changes
7. Create a pull request

### Debugging

*Document debugging tools and techniques:*
- Logging approach
- Debugging tools
- Common issues and solutions

---

## Project-Specific Guidelines

*Add any project-specific conventions, patterns, or requirements here as they emerge:*

### Architecture Patterns
[Document architectural decisions and patterns]

### Dependencies
[List key dependencies and their purposes]

### Performance Considerations
[Document any performance requirements or optimizations]

### Accessibility
[Document accessibility requirements and standards]

### Internationalization
[Document i18n approach if applicable]

---

## Updating This Document

This document should be treated as living documentation:

- **When to Update**:
  - New major features are added
  - Architecture changes
  - New conventions are established
  - Development workflow changes
  - New tools or technologies are adopted

- **Who Should Update**:
  - Any developer or AI assistant working on the project
  - Update relevant sections when making significant changes

- **Review Process**:
  - Include CLAUDE.md updates in pull requests when applicable
  - Review for accuracy and completeness periodically

---

## Additional Resources

*Add links to relevant resources:*

- [Project documentation]
- [API documentation]
- [Design system]
- [Contributing guidelines]
- [Code of conduct]

---

## Questions or Issues?

If you encounter issues or have questions about conventions not covered here:

1. Search the codebase for similar patterns
2. Check git history for context on decisions
3. Ask the project maintainers
4. Update this document once clarification is received

---

**Last Updated**: 2025-11-21
**Status**: Initial version for new repository
**Next Review**: After first major milestone or 30 days
